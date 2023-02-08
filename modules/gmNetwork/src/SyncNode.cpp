
#include <gmNetwork/SyncNode.hh>

#include <gmCore/ExitException.hh>
#include <gmCore/RunOnce.hh>
#include <gmCore/InvalidArgument.hh>
#include <gmCore/Console.hh>
#include <gmCore/Stringify.hh>

#include <unordered_map>
#include <limits>
#include <functional>

#define ASIO_STANDALONE
#include <asio.hpp>
#undef ASIO_STANDALONE

BEGIN_NAMESPACE_GMNETWORK;

GM_OFI_DEFINE(SyncNode);
GM_OFI_PARAM2(SyncNode, peer, std::string, addPeer);
GM_OFI_PARAM2(SyncNode, localPeerIdx, size_t, setLocalPeerIdx);
GM_OFI_PARAM2(SyncNode, exitWhenAPeerIsDisconnected, bool, setExitWhenAPeerIsDisconnected);
GM_OFI_PARAM2(SyncNode, timeoutDelay, float, setTimeoutDelay);

#define DEFAULT_SERVICE "20401"
#define RECONNECT_DELAY std::chrono::seconds(1)
#define PROTOCOL_ID_HANDSHAKE 1
#define PROTOCOL_ID_PING      2
#define PROTOCOL_ID_PONG      3

struct SyncNode::Impl : public std::enable_shared_from_this<SyncNode::Impl> {

  /**
     A connection and communication handler for one single peer.
  */
  struct Peer
    : std::enable_shared_from_this<Peer> {

    /**
       Create a beta node.
    */
    Peer (asio::io_context &io_context,
          SyncNode::Impl *parent,
          std::string address,
          asio::ip::tcp::resolver::results_type endpoints,
          std::size_t local_peer_idx,
          std::size_t peer_idx)
      : io_context(io_context),
        parent(parent),
        socket(io_context),
        timeout_timer(io_context),
        pingpong_timer(io_context),
        address(address),
        endpoints(endpoints),
        local_peer_idx(local_peer_idx),
        peer_idx(peer_idx) {

      // Beta peer

    }

    Peer (asio::io_context &io_context,
          SyncNode::Impl *parent,
          asio::ip::tcp::socket socket,
          std::size_t local_peer_idx)
      : io_context(io_context),
        parent(parent),
        socket(std::move(socket)),
        timeout_timer(io_context),
        pingpong_timer(io_context),
        local_peer_idx(local_peer_idx),
        peer_idx(std::numeric_limits<std::size_t>::max()) {

      // Alpha peer

    }

    void connect();
    void on_connect(std::error_code ec, asio::ip::tcp::endpoint end);
    void on_data(std::error_code ec,
                 std::shared_ptr<std::vector<char>> read_buffer,
                 std::size_t length);
    void on_write(std::error_code ec, std::size_t length);
    void setup_pingpong_timer();
    void setup_timeout_timer();
    void reset_timers();
    void on_pingpong_timeout();
    void on_timeout_timeout();

    bool isConnected();

    void sendMessage(Protocol::Message mess);
    void readData();

    std::size_t getPeerIdx() { return peer_idx; }

  private:

    asio::io_context &io_context;
    SyncNode::Impl *parent;
    asio::ip::tcp::socket socket;
    bool is_connected = false;
    asio::steady_timer timeout_timer;
    asio::steady_timer pingpong_timer;
    std::mutex peer_lock;

    Protocol::Message message;

    std::string address;
    asio::ip::tcp::resolver::results_type endpoints;
    std::size_t local_peer_idx;
    std::size_t peer_idx;
  };

  ~Impl();

  static void split_address_service(std::string comb, std::string &host, std::string &port);

  bool initialize();

  void addPeer(std::string address);
  void setLocalPeerIdx(size_t idx);
  size_t getLocalPeerIdx();
  std::size_t getPeersCount();
  std::set<std::size_t> getConnectedPeers();
  float getTimeoutDelay();

  void waitForConnection();
  bool isConnected();

  void sendMessage(Protocol::Message mess);
  void addProtocol(std::string name, std::shared_ptr<Protocol> prot);

  void runContext();
  void accept();
  void on_accept(std::error_code ec, asio::ip::tcp::socket socket);
  void lost_connection(Peer * peer);

  void routeMessage(Protocol::Message mess);

  asio::io_context io_context;
  std::thread io_thread;
  std::mutex impl_lock;

  std::vector<std::shared_ptr<Peer>> alpha_peers;
  std::vector<std::shared_ptr<Peer>> beta_peers;
  std::vector<std::string> peer_addresses;
  std::size_t local_peer_idx = std::numeric_limits<size_t>::max();
  bool exit_when_a_peer_is_disconnected = false;
  float timeout_delay = 5.f;
  std::set<std::size_t> connected_peers;

  std::unique_ptr<asio::ip::tcp::acceptor> server_acceptor;

  std::vector<Protocol::Message> unprocessed_messages;
  std::unordered_map<char, std::shared_ptr<Protocol>> protocols;
  std::unordered_map<std::string, char> protocol_id_by_name;

  std::condition_variable waiting_condition;
};


SyncNode::Impl::~Impl() {

  GM_DBG1("SyncNode", local_peer_idx << " Closing context");
  io_context.stop();

  if (io_thread.joinable())
    try {
      io_thread.join();
      GM_DBG2("SyncNode", local_peer_idx << " Successfully waited for io thread to stop.");
    } catch (const std::invalid_argument &e) {
      GM_DBG2("SyncNode",
              local_peer_idx << " Could not join io thread: " << e.what()
                             << ".");
    } catch (const std::system_error &e) {
      GM_WRN("SyncNode",
             local_peer_idx
                 << " Caught system_error while joining IO thread. Code "
                 << e.code() << " meaning " << e.what() << ".");
    }

  {
    std::unique_lock<std::mutex> guard(impl_lock);

    auto protocols_tmp = protocols;
    protocols.clear();
    guard.unlock();

    for (auto protocol : protocols_tmp)
      protocol.second->setSyncNode(nullptr);
  }
  protocol_id_by_name.clear();
}

SyncNode::SyncNode()
  : _impl(std::make_unique<Impl>()) {}

SyncNode::~SyncNode() {}

void SyncNode::addPeer(std::string address) {
  if (isInitialized())
    throw gmCore::PreConditionViolation("Adding peer after initialization is not supported");
  _impl->addPeer(address);
}

void SyncNode::Impl::addPeer(std::string address) {
  std::lock_guard<std::mutex> guard(impl_lock);
  if (peer_addresses.size() >= 255)
    throw gmCore::PreConditionViolation("Support only for 255 peers.");
  peer_addresses.push_back(address);
}

void SyncNode::setLocalPeerIdx(size_t idx) {
  if (isInitialized())
    throw gmCore::PreConditionViolation("Setting local peer after initialization is not supported");
  _impl->setLocalPeerIdx(idx);
}

void SyncNode::Impl::setLocalPeerIdx(size_t idx) {
  std::lock_guard<std::mutex> guard(impl_lock);
  local_peer_idx = idx;
}

size_t SyncNode::getLocalPeerIdx() {
  return _impl->getLocalPeerIdx();
}

size_t SyncNode::Impl::getLocalPeerIdx() {
  std::lock_guard<std::mutex> guard(impl_lock);
  return local_peer_idx;
}

void SyncNode::setExitWhenAPeerIsDisconnected(bool on) {
  std::lock_guard<std::mutex> guard(_impl->impl_lock);
  _impl->exit_when_a_peer_is_disconnected = on;
}

void SyncNode::setTimeoutDelay(float t) {
  std::lock_guard<std::mutex> guard(_impl->impl_lock);
  _impl->timeout_delay = t;
}

float SyncNode::getTimeoutDelay() {
  return _impl->getTimeoutDelay();
}

float SyncNode::Impl::getTimeoutDelay() {
  std::lock_guard<std::mutex> guard(impl_lock);
  return timeout_delay;
}

std::size_t SyncNode::getPeersCount() {
  return _impl->getPeersCount();
}

std::size_t SyncNode::Impl::getPeersCount() {
  std::lock_guard<std::mutex> guard(impl_lock);
  return peer_addresses.size() - 1;
}

std::set<std::size_t> SyncNode::getConnectedPeers() {
  return _impl->getConnectedPeers();
}

std::set<std::size_t> SyncNode::Impl::getConnectedPeers() {

  std::set<std::size_t> peers;

  std::lock_guard<std::mutex> guard(impl_lock);

  for (auto peer : alpha_peers)
    if (peer->isConnected()) {
      peers.insert(peer->getPeerIdx());
      connected_peers.insert(peer->getPeerIdx());
    } else {
      if (exit_when_a_peer_is_disconnected) {
        throw gmCore::ExitException(0);
      } else {
        connected_peers.erase(peer->getPeerIdx());
      }
    }

  for (auto peer : beta_peers)
    if (peer->isConnected()) {
      peers.insert(peer->getPeerIdx());
      connected_peers.insert(peer->getPeerIdx());
    } else {
      if (exit_when_a_peer_is_disconnected) {
        throw gmCore::ExitException(0);
      } else {
        connected_peers.erase(peer->getPeerIdx());
      }
    }

  return peers;
}

void SyncNode::Impl::split_address_service
(std::string comb, std::string &host, std::string &port) {

  if (comb.find_first_of(":") != comb.find_last_of(":")) {
    // IPv6

    std::size_t sep_pos = comb.find("]:");
    if (sep_pos != std::string::npos && sep_pos + 2 < comb.size()) {

      if (comb[0] != '[')
        throw gmCore::InvalidArgument("Incorrectly formatted IPv6 address");

      host = comb.substr(1, sep_pos - 1);
      port = comb.substr(sep_pos + 2);

    } else {
      host = comb;
      port = DEFAULT_SERVICE;
    }

  } else {
    // IPv4

    std::size_t sep_pos = comb.find(":");
    if (sep_pos != std::string::npos && sep_pos + 1 < comb.size()) {

      host = comb.substr(0, sep_pos);
      port = comb.substr(sep_pos + 1);

    } else {
      host = comb;
      port = DEFAULT_SERVICE;
    }
  }
}

void SyncNode::Impl::Peer::connect() {

  std::lock_guard<std::mutex> guard(peer_lock);

  GM_DBG2("SyncNode", local_peer_idx << " Trying to connect " << address);
  asio::async_connect(socket, endpoints,
                      [this](std::error_code ec, asio::ip::tcp::endpoint end) {
                        on_connect(ec, end);
                      });
}

void SyncNode::Impl::Peer::on_connect(std::error_code ec,
                                      asio::ip::tcp::endpoint) {

  std::unique_lock<std::mutex> guard(peer_lock);

  if (ec) {
    GM_DBG2("SyncNode", local_peer_idx << " Failed to connect to " << peer_idx << ": " << ec.message());

    timeout_timer.async_wait(std::bind(&Peer::connect, this));
    timeout_timer.expires_from_now(RECONNECT_DELAY);

    return;
  }

  GM_DBG1("SyncNode", local_peer_idx << " Connected to " << peer_idx << " - sending handshake");

  is_connected = true;

  guard.unlock();

  {
    std::unique_lock<std::mutex> guard(parent->impl_lock);
    parent->waiting_condition.notify_all();
  }

  setup_pingpong_timer();
  setup_timeout_timer();
  reset_timers();

  // Handshake
  sendMessage(Protocol::Message(PROTOCOL_ID_HANDSHAKE,
                                { (char)parent->getLocalPeerIdx(),
                                  (char)GRAMODS_NETWORK_VERSION }));

  readData();
}

void SyncNode::Impl::Peer::readData() {

  std::lock_guard<std::mutex> guard(peer_lock);
  if (!socket.is_open()) return;

  std::size_t to_read = message.length ?
    message.length - message.data.size() : message.getHeader().size();

  std::shared_ptr<std::vector<char>> read_buffer =
    std::make_shared<std::vector<char>>(to_read, 0);

  asio::async_read(socket,
                   asio::buffer(read_buffer->data(),
                                read_buffer->size()),
                   [this, read_buffer]
                   (std::error_code ec, std::size_t length) {
                     on_data(ec, read_buffer, length);
                   });
}

void SyncNode::Impl::Peer::on_data
(std::error_code ec,
 std::shared_ptr<std::vector<char>> read_buffer,
 std::size_t length) {

  if (ec) {

    std::unique_lock<std::mutex> guard(peer_lock);

    GM_WRN("SyncNode", local_peer_idx << " Incoming data problem for " << peer_idx << " (" << ec.message() << ")");

    is_connected = false;
    socket.close();

    guard.unlock();

    parent->lost_connection(this);

    return;
  }

  reset_timers();

  std::unique_lock<std::mutex> guard(peer_lock);

  GM_DBG3("SyncNode", local_peer_idx << " Got data from " << peer_idx << " (len = " << length << ")");

  if (message.length) {
    // In process of reading message data

    message.data.insert(message.data.end(),
                        read_buffer->begin(),
                        read_buffer->end());

    if (message.data.size() == message.length) {

      switch (message.protocol) {

      case 0:
        GM_WRN("SyncNode", local_peer_idx << " Got message without protocol id (" << message.data.size() << ").");
        break;

      case PROTOCOL_ID_HANDSHAKE: {

        if (peer_idx != std::numeric_limits<size_t>::max()) {
          GM_WRN("SyncNode", local_peer_idx << " Got handshake message (size " << message.data.size() << ") from " << peer_idx << " after already estabilishing connection.");
          break;
        }

        if (message.data.size() != 2) {
          GM_ERR("SyncNode", local_peer_idx << " Corrupt handshake or incompatible networking versions");
          socket.close();
          break;
        }

        peer_idx = message.data[0];
        char comm_ver = message.data[1];

        if (comm_ver != GRAMODS_NETWORK_VERSION) {
          GM_ERR("SyncNode", local_peer_idx << " Incompatible networking versions");
          socket.close();
          break;
        }

        is_connected = true;

        guard.unlock();

        std::unique_lock<std::mutex> impl_guard(parent->impl_lock);
        parent->waiting_condition.notify_all();

        guard.lock();

        GM_DBG1("SyncNode", local_peer_idx << " Connection with " << peer_idx << " established with handshake.");

      }
        break;

      case PROTOCOL_ID_PING: {

        guard.unlock();

        if (message.data.size() != 1)
          GM_WRN("SyncNode", "Received corrupt PING message");
        else if (peer_idx != size_t(message.data[0]))
          GM_WRN("SyncNode", "Received PING message with confusing payload");

        GM_DBG2("SyncNode", local_peer_idx << " Sending pong to " << peer_idx);

        // Pong
        Protocol::Message msg(PROTOCOL_ID_PONG,
                              { (char)parent->getLocalPeerIdx() });
        msg.to_peer_idx = peer_idx;
        sendMessage(msg);

        guard.lock();
      }
        break;

      case PROTOCOL_ID_PONG:
        // Ignore pong message - already reset the timeout timer when
        // we got data (above)
        break;

      default: {

        GM_DBG3("SyncNode", local_peer_idx << " Message from " << peer_idx << " complete (len = " << message.data.size() << ")");
        guard.unlock();

        parent->routeMessage(message);

        guard.lock();
      }
      }
      
      message.length = 0;

    } else {
      GM_WRN("SyncNode", local_peer_idx << " This should not happen: message incomplete (got " << message.data.size() << " of " << message.length << ")");
    }

  } else {
    // In process of reading message header
    message = Protocol::Message(*read_buffer);
    message.from_peer_idx = peer_idx;
    GM_DBG3("SyncNode", local_peer_idx << " RECV"
            << " peer=" << message.from_peer_idx
            << " protocol=" << (int)message.protocol
            << " length=" << message.length << " bytes");
  }

  guard.unlock();

  readData();
}

void SyncNode::sendMessage(Protocol::Message mess) {
  _impl->sendMessage(mess);
}

void SyncNode::Impl::sendMessage(Protocol::Message mess) {

  GM_DBG3("SyncNode", local_peer_idx << " Sending message "
           "(from = " << local_peer_idx <<
           ", type = " << (int)mess.protocol <<
           ", len = " << mess.data.size() << ")");


  std::unique_lock<std::mutex> guard(impl_lock);

  std::vector<std::shared_ptr<Peer>> rcp_peers;
  rcp_peers.reserve(alpha_peers.size() + beta_peers.size());

  if (mess.to_peer_idx == std::numeric_limits<size_t>::max()) {

    for (auto peer : alpha_peers)
      rcp_peers.push_back(peer);
    for (auto peer : beta_peers)
      rcp_peers.push_back(peer);

  } else if (mess.to_peer_idx < local_peer_idx) {
    rcp_peers.push_back(alpha_peers[mess.to_peer_idx]);
  } else if (mess.to_peer_idx > local_peer_idx) {
    rcp_peers.push_back(beta_peers[mess.to_peer_idx - local_peer_idx - 1]);
  } else {
    GM_WRN("SyncNode", local_peer_idx << " Trying to send message to peer idx " << mess.to_peer_idx << " that is not a peer.");
  }

  guard.unlock();

  for (auto peer : rcp_peers)
    peer->sendMessage(mess);
}

void SyncNode::Impl::routeMessage(Protocol::Message mess) {

  std::unique_lock<std::mutex> guard(impl_lock);

  if (protocols.count(mess.protocol) > 0) {
    std::shared_ptr<Protocol> & protocol = protocols[mess.protocol];
    guard.unlock();
    protocol->processMessage(mess);
    return;
  }

  if (unprocessed_messages.size() < 100)
    unprocessed_messages.push_back(mess);
  else
    GM_RUNONCE(GM_ERR("SyncNode", local_peer_idx << " Unknown message type (" << (int)mess.protocol << ") from " << mess.from_peer_idx << "."));
}

bool SyncNode::Impl::Peer::isConnected() {
  std::lock_guard<std::mutex> guard(peer_lock);
  return is_connected;
}

void SyncNode::Impl::Peer::sendMessage(Protocol::Message mess) {

  if (!socket.is_open()) return;

  std::lock_guard<std::mutex> guard(peer_lock);

  std::weak_ptr<Peer> weak_this(shared_from_this());

  std::shared_ptr<std::vector<char>> buffer =
    std::make_shared<std::vector<char>>();
  buffer->reserve(Protocol::HEADER_LENGTH + mess.data.size());
  *buffer = mess.getHeader();
  buffer->insert(buffer->end(), mess.data.begin(), mess.data.end());

  asio::async_write(socket,
                    asio::buffer(buffer->data(), buffer->size()),
                    [weak_this, buffer]
                    (std::error_code ec, std::size_t length) {
                      auto _this = weak_this.lock();
                      if (_this) _this->on_write(ec, length);
                    });
}

void SyncNode::Impl::Peer::on_write(std::error_code ec,
                                    std::size_t length) {

  std::unique_lock<std::mutex> guard(peer_lock);

  if (ec) {

    GM_ERR("SyncNode", local_peer_idx << " Could not write - closing socket (" << ec.message() << ")");

    for (auto end : endpoints)
      GM_ERR("SyncNode", local_peer_idx << " (" << end.host_name() << ":" << end.service_name() << ")");

    is_connected = false;
    socket.close();

    guard.unlock();

    parent->lost_connection(this);

  } else {
    GM_DBG3("SyncNode", local_peer_idx << " Successfully sent " << length << " bytes");
  }
}

void SyncNode::Impl::Peer::setup_pingpong_timer() {

  std::weak_ptr<Peer> weak_this(shared_from_this());

  std::lock_guard<std::mutex> guard(peer_lock);

  pingpong_timer.async_wait([weak_this](const std::error_code &) {
    auto _this = weak_this.lock();
    if (_this) _this->on_pingpong_timeout();
  });
}

void SyncNode::Impl::Peer::setup_timeout_timer() {

  std::weak_ptr<Peer> weak_this(shared_from_this());

  std::lock_guard<std::mutex> guard(peer_lock);

  timeout_timer.async_wait([weak_this](const std::error_code &) {
    auto _this = weak_this.lock();
    if (_this) _this->on_timeout_timeout();
  });
}

void SyncNode::Impl::Peer::reset_timers() {

  float delay = parent->getTimeoutDelay();

  std::lock_guard<std::mutex> guard(peer_lock);

  typedef std::chrono::duration<float, std::ratio<1>> f_seconds;
  typedef asio::steady_timer::clock_type::duration timer_duration;

  timeout_timer.expires_after
    (std::chrono::duration_cast<timer_duration>(f_seconds(delay)));
  pingpong_timer.expires_after
    (std::chrono::duration_cast<timer_duration>(f_seconds(0.5f * delay)));
}

void SyncNode::Impl::Peer::on_pingpong_timeout() {

  if (!socket.is_open()) return;

  setup_pingpong_timer();

  std::unique_lock<std::mutex> guard(peer_lock);

  if (pingpong_timer.expiry() > asio::steady_timer::clock_type::now())
    return;

  guard.unlock();
  float delay = parent->getTimeoutDelay();
  guard.lock();

  GM_DBG2("SyncNode", local_peer_idx << " Sending ping to " << peer_idx);

  typedef std::chrono::duration<float, std::ratio<1>> f_seconds;
  typedef asio::steady_timer::clock_type::duration timer_duration;

  pingpong_timer.expires_after
    (std::chrono::duration_cast<timer_duration>(f_seconds(0.5f * delay)));

  guard.unlock();

  // Ping
  sendMessage(Protocol::Message(PROTOCOL_ID_PING,
                                { (char)parent->getLocalPeerIdx() }));
}

void SyncNode::Impl::Peer::on_timeout_timeout() {

  if (!socket.is_open()) return;

  if (timeout_timer.expiry() > asio::steady_timer::clock_type::now()) {
    setup_timeout_timer();
    return;
  }

  std::unique_lock<std::mutex> guard(peer_lock);

  GM_WRN("SyncNode", local_peer_idx << " Connection timeout for " << peer_idx);

  is_connected = false;
  socket.close();

  guard.unlock();

  parent->lost_connection(this);
}

void SyncNode::initialize() {

  if (isInitialized())
    return;

  if (_impl->initialize())
    Object::initialize();
}

bool SyncNode::Impl::initialize() {
  std::lock_guard<std::mutex> guard(impl_lock);

  if (peer_addresses.size() == 0) {
    GM_ERR("SyncNode", local_peer_idx << " Cannot connect - no addresses");
    return false;
  }

  if (local_peer_idx >= peer_addresses.size()) {
    GM_ERR("SyncNode", local_peer_idx << " Invalid local peer idx, " << local_peer_idx);
    throw gmCore::PreConditionViolation("invalid local peer idx");
  }

  asio::ip::tcp::resolver resolver(io_context);

  for (size_t idx = local_peer_idx + 1; idx < peer_addresses.size(); ++idx) {
    std::string address = peer_addresses[idx];
    GM_DBG2("SyncNode", local_peer_idx << " Adding Beta Peer " << idx << " (" << address << ")");

    std::string host;
    std::string port;
    split_address_service(address, host, port);

    auto endpoints = resolver.resolve(host, port);
    if (endpoints.empty()) {
      GM_ERR("SyncNode", local_peer_idx << " Cannot resolve " << host << ":" << port);
      throw gmCore::InvalidArgument("Cannot resolve peer address");
    }

    beta_peers.push_back(std::make_shared<Peer>(io_context,
                                                this,
                                                address,
                                                endpoints,
                                                local_peer_idx,
                                                idx));
  }

  asio::ip::tcp::resolver::results_type bind_endpoints;
  {
    std::string address = peer_addresses[local_peer_idx];
    GM_DBG1("SyncNode", local_peer_idx << " Bind address " << address);

    std::string host;
    std::string port;
    split_address_service(address, host, port);

    bind_endpoints = resolver.resolve(host, port);

    for (auto end : bind_endpoints)
      GM_DBG2("SyncNode", local_peer_idx << " Resolved "
             << end.host_name()
             << " : "
             << end.service_name()
             << " for tcp acceptor.");

    server_acceptor =
      std::make_unique<asio::ip::tcp::acceptor>
      (io_context, *bind_endpoints.begin());
  }

  if (alpha_peers.size() < local_peer_idx)
    accept();

  for (auto peer : beta_peers) {
    peer->connect();
  }

  io_thread = std::thread([this](){ this->runContext(); });

  return true;
}

void SyncNode::Impl::accept() {

  GM_DBG1("SyncNode", local_peer_idx << " Listening to incoming connections");
  server_acceptor->async_accept
    ([this] (std::error_code ec, asio::ip::tcp::socket socket) {
       on_accept(ec, std::move(socket));
     });
}

void SyncNode::Impl::on_accept(std::error_code ec, asio::ip::tcp::socket socket) {

  if (ec) {
    GM_WRN("SyncNode", local_peer_idx << " Incoming connection problem (" << ec.message() << ")");
    accept();
    return;
  }

  auto endpoint = socket.remote_endpoint();

  GM_DBG1("SyncNode", local_peer_idx << " Received incoming connection (" << endpoint.address() << ":" << endpoint.port() << ")");

  socket.set_option(asio::ip::tcp::no_delay(true));

  std::shared_ptr<Peer> peer(std::make_shared<Peer>(io_context,
                                                    this,
                                                    std::move(socket),
                                                    local_peer_idx));
  {
    std::lock_guard<std::mutex> guard(impl_lock);

    alpha_peers.push_back(peer);

    if (alpha_peers.size() < local_peer_idx)
      accept();
  }

  peer->setup_pingpong_timer();
  peer->setup_timeout_timer();
  peer->reset_timers();

  peer->readData();

}

void SyncNode::Impl::lost_connection(Peer * peer) {

  std::size_t idx = peer->getPeerIdx();
  if (idx == std::numeric_limits<size_t>::max())
    return;

  std::unique_lock<std::mutex> guard(impl_lock);
  auto _protocols = protocols;
  guard.unlock();

  for (auto protocol : _protocols)
    protocol.second->lostPeer(idx);
}

void SyncNode::waitForConnection() {
  _impl->waitForConnection();
}

void SyncNode::Impl::waitForConnection() {

  std::unique_lock<std::mutex> guard(impl_lock);

  GM_DBG1("SyncNode", local_peer_idx << " Waiting for all peers to connect.");

  while (true) {

    if (io_context.stopped()) {
      GM_DBG1("SyncNode", local_peer_idx << " IO context stopped while waiting for connection.");
      return;
    }

    std::size_t connect_count = 0;
    std::stringstream ss;

    for (auto peer : alpha_peers) {
      if (peer->isConnected())
        ++connect_count;
      ss << (peer->isConnected() ? "1" : "0");
    }

    ss << "X";
    for (auto peer : beta_peers) {
      if (peer->isConnected())
        ++connect_count;
      ss << (peer->isConnected() ? "1" : "0");
    }

    guard.unlock();

    GM_DBG1("SyncNode", local_peer_idx << " Connection wait status is " << ss.str()
           << " of " << getPeersCount() << ".");

    assert(connect_count <= getPeersCount());
    if (connect_count == getPeersCount())
      break;

    guard.lock();

    waiting_condition.wait_for(guard, std::chrono::seconds(1));
  }

  GM_DBG1("SyncNode", local_peer_idx << " All peers connected");
}

bool SyncNode::isConnected() {
  return _impl->isConnected();
}

bool SyncNode::Impl::isConnected() {

  std::unique_lock<std::mutex> guard(impl_lock);

  GM_DBG2("SyncNode", local_peer_idx << " Checking if all peers are connected.");

  if (io_context.stopped()) {
    GM_DBG1("SyncNode", local_peer_idx << " IO context stopped when checking if all are connected.");
    return true;
  }

  std::size_t connect_count = 0;
  std::stringstream ss;

  for (auto peer : alpha_peers) {
    if (peer->isConnected())
      ++connect_count;
    ss << (peer->isConnected() ? "1" : "0");
  }
  ss << "X";
  for (auto peer : beta_peers) {
    if (peer->isConnected())
        ++connect_count;
    ss << (peer->isConnected() ? "1" : "0");
  }

  guard.unlock();

  assert(connect_count <= getPeersCount());
  if (connect_count < getPeersCount()) {
    GM_DBG2("SyncNode", local_peer_idx << " Connection wait status is " << ss.str()
            << " of " << getPeersCount() << ".");
    return false;
  }

  GM_DBG1("SyncNode", local_peer_idx << " All peers connected");
  return true;
}

void SyncNode::Impl::runContext() {
  while (true) {
    try {
      GM_DBG1("SyncNode", local_peer_idx << " Running ASIO io context");
      io_context.run();
      break;
    }
    catch (const std::exception &e) {
      GM_WRN("SyncNode", local_peer_idx << " Exception caught from ASIO io context: " << e.what());
    }
  }
  GM_DBG1("SyncNode", local_peer_idx << " Done running ASIO io context");
}

Protocol * SyncNode::getProtocol(std::string name) {
  if (_impl->protocol_id_by_name.count(name) > 0) {
    return getProtocol(_impl->protocol_id_by_name[name]);
  } else {
    return nullptr;
  }
}

Protocol * SyncNode::getProtocol(char id) {
  if (_impl->protocols.count(id) > 0) {
    return _impl->protocols[id].get();
  } else {
    return nullptr;
  }
}

void SyncNode::addProtocol(std::string name, std::shared_ptr<Protocol> prot) {
  prot->setSyncNode(this);
  return _impl->addProtocol(name, prot);
}

void SyncNode::Impl::addProtocol(std::string name, std::shared_ptr<Protocol> prot) {

  std::unique_lock<std::mutex> guard(impl_lock);

  std::vector<Protocol::Message> messages_to_process;
  messages_to_process.reserve(unprocessed_messages.size());

  if (protocol_id_by_name.count(name) > 0)
    throw gmCore::PreConditionViolation
      (GM_STR("There is already a protocol named " << name));

  char id = prot->getProtocolFlag();
  if (id < 10)
    throw gmCore::PreConditionViolation
      (GM_STR("Protocol id " << id << " is reserved for internal protocols"));
  if (protocols.count(id) > 0)
    throw gmCore::PreConditionViolation
      (GM_STR("There is already a protocol with id " << id));

  protocols[id] = prot;
  protocol_id_by_name[name] = id;

  if (unprocessed_messages.empty()) return;

  std::vector<Protocol::Message> still_unprocessed_messages;
  still_unprocessed_messages.reserve(unprocessed_messages.size());

  for (auto msg : unprocessed_messages)
    if (msg.protocol == id)
      messages_to_process.push_back(msg);
    else
      still_unprocessed_messages.push_back(msg);

  unprocessed_messages.swap(still_unprocessed_messages);

  guard.unlock();

  for (auto msg : messages_to_process)
    prot->processMessage(msg);
}

END_NAMESPACE_GMNETWORK;
