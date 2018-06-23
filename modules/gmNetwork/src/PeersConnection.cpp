
#include <gmNetwork/PeersConnection.hh>

BEGIN_NAMESPACE_GMNETWORK;

GM_OFI_DEFINE(PeersConnection);
GM_OFI_PARAM(PeersConnection, peer, std::string, PeersConnection::addPeer);
GM_OFI_PARAM(PeersConnection, localPeerIdx, int, PeersConnection::setLocalPeerIdx);

#define DEFAULT_SERVICE "20401"
#define HANDSHAKE_LENGTH 256
#define HEADER_LENGTH (1+1+4)
#define FIRST_CONNECT_DELAY_MS boost::posix_time::milliseconds(20)
#define RECONNECT_DELAY_MS boost::posix_time::milliseconds(2000)

PeersConnection::PeersConnection()
  : closing(false) {}

PeersConnection::~PeersConnection() {
  close();
  io_thread.join();
}

void PeersConnection::close() {
  if (closing) return;
  closing = true;

  GM_INF("PeersConnection", "Closing protocols");
  for (auto protocol : protocols) {
    auto _protocol = protocol.lock();
    if (_protocol)
      _protocol->close();
  }
  protocols.clear();

  GM_INF("PeersConnection", "Closing context");
  io_context.stop();

  closing = false;
}

void PeersConnection::addPeer(std::string address) {
  if (isInitialized())
    throw std::logic_error("Adding peer after initialization is not supported");
  peer_addresses.push_back(address);
}

void PeersConnection::setLocalPeerIdx(int idx) {
  if (isInitialized())
    throw std::logic_error("Setting local peer after initialization is not supported");

  local_peer_idx = idx;
}

int PeersConnection::getLocalPeerIdx() {
  return local_peer_idx;
}

std::size_t PeersConnection::getPeersCount() {
  return peer_addresses.size();
}

void PeersConnection::split_address_service
(std::string comb, std::string &host, std::string &port) {

  if (comb.find_first_of(":") != comb.find_last_of(":")) {
    // IPv6

    std::size_t sep_pos = comb.find("]:");
    if (sep_pos != std::string::npos && sep_pos + 2 < comb.size()) {

      if (comb[0] != '[')
        throw std::invalid_argument("Incorrectly formatted IPv6 address");

      host = comb.substr(1, sep_pos);
      port = comb.substr(sep_pos + 2);

    } else {
      host = comb;
      port = DEFAULT_SERVICE;
    }

    GM_INF("PeersConnection", "IPv6 address '" << comb << "' = " << host << " : " << port);

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

    GM_INF("PeersConnection", "IPv4 address '" << comb << "' = " << host << " : " << port);
  }
}

bool PeersConnection::Peer::connect() {

  std::weak_ptr<Peer> weak_self(shared_from_this());

  GM_VINF("PeersConnection", "Trying to connect " << address);
  asio::async_connect(socket, endpoints,
                      [weak_self](std::error_code ec, asio::ip::tcp::endpoint end) {

                        auto self = weak_self.lock();
                        if (!self) return;

                        if (!ec) {

                          GM_VINF("PeersConnection", "Connected");

                          self->socket.set_option(asio::ip::tcp::no_delay(true));
                          self->is_connected = true;
                          self->sendHandshake();

                        } else {

                          GM_VINF("PeersConnection", "Could not connect: " << ec.message());

                          auto ptr = std::make_shared<asio::deadline_timer>
                            (self->io_context);
                          ptr->expires_from_now(RECONNECT_DELAY_MS);

                          ptr->async_wait([self, ptr](const std::error_code&){
                              self->connect();
                            });
                        }
                      });
}

void PeersConnection::Peer::sendHandshake() {

  std::stringstream string_data;
  string_data << GRAMODS_NETWORK_VERSION << " " << address;

  std::vector<char> data(HANDSHAKE_LENGTH, 0);
  assert(string_data.str().size() < HANDSHAKE_LENGTH - 1);
  memcpy(data.data(), string_data.str().c_str(),
         sizeof(char) * string_data.str().size());

  GM_INF("PeersConnection", "Sending handshake '" << string_data.str() << "'");
  int N = asio::write(socket, asio::buffer(data, HANDSHAKE_LENGTH));
}

void PeersConnection::Peer::readHandshake() {

  std::weak_ptr<Peer> weak_self(shared_from_this());

  std::shared_ptr<std::vector<char>> buffer =
    std::make_shared<std::vector<char>>(HANDSHAKE_LENGTH, 0);

  asio::async_read(socket,
                   asio::buffer(buffer->data(), buffer->size()),
                   [weak_self, buffer](std::error_code ec, std::size_t length){

                     auto self = weak_self.lock();
                     if (!self) return;

                     if (ec)
                       GM_WRN("PeersConnection", "Incoming connection problem (" << ec << ")");

                     std::string data(buffer->begin(), buffer->end());

                     std::string PeersConnection_version;
                     std::stringstream(data) >> PeersConnection_version >> self->address;

                     GM_INF("PeersConnection",
                            "Got handshake: " << length
                            << " of " << buffer->size()
                            << " (version=" << PeersConnection_version
                            << ", address=" << self->address << ")");

                     self->is_connected = true;
                   });
}

void PeersConnection::Peer::readData() {

  std::weak_ptr<Peer> weak_self(shared_from_this());

  std::size_t to_read = message ? message_length - message->data.size() : HEADER_LENGTH;
    
  std::shared_ptr<std::vector<char>> read_buffer =
    std::make_shared<std::vector<char>>(to_read, 0);

  asio::async_read(socket,
                   asio::buffer(read_buffer->data(),
                                read_buffer->size()),
                   [weak_self, read_buffer, to_read]
                   (std::error_code ec,
                    std::size_t length) {

                     auto self = weak_self.lock();
                     if (!self) return;

                     GM_VVINF("PeersConnection",
                              "Got message (" << length << ")");

                     assert(length == to_read);

                     if (self->message) {
                       // In process of reading message data

                       self->message->data.insert(self->message->data.end(),
                                                  read_buffer->begin(),
                                                  read_buffer->end());

                       if (self->message->data.size() == self->message_length) {
                         // Send message to protocol
                         self->message = nullptr;
                         self->message_length = 0;
                       }
                       
                     } else {
                       // In process of reading message header

                       self->message = std::make_unique<Protocol::Message>();
                       self->message->peer_idx = (*read_buffer)[0];
                       self->message->protocol = (*read_buffer)[1];
                       self->message_length = (((*read_buffer)[2] <<  0) +
                                               ((*read_buffer)[3] <<  8) +
                                               ((*read_buffer)[4] << 16) +
                                               ((*read_buffer)[5] << 24));
                     }

                     self->readData();
                   });
}

void PeersConnection::sendMessage(Protocol::Message mess) {
  mess.peer_idx = local_peer_idx;
  for (auto peer : alpha_peers)
    peer->sendMessage(mess);
  for (auto peer : beta_peers)
    peer->sendMessage(mess);
}

void PeersConnection::Peer::sendMessage(Protocol::Message mess) {

  std::weak_ptr<Peer> weak_self(shared_from_this());

  std::shared_ptr<std::vector<char>> buffer =
    std::make_shared<std::vector<char>>();
  buffer->reserve(HEADER_LENGTH + mess.data.size());

  buffer->push_back(mess.peer_idx);
  buffer->push_back(mess.protocol);
  buffer->push_back(0xff & (mess.data.size() >>  0));
  buffer->push_back(0xff & (mess.data.size() >>  8));
  buffer->push_back(0xff & (mess.data.size() >> 16));
  buffer->push_back(0xff & (mess.data.size() >> 24));
  buffer->insert(buffer->end(), mess.data.begin(), mess.data.end());

  asio::async_write(socket,
                    asio::buffer(buffer->data(), buffer->size()),
                    [weak_self, buffer](std::error_code ec, std::size_t) {

                      auto self = weak_self.lock();
                      if (!self) return;

                      if (ec) {
                        GM_ERR("PeersConnection", "Could not write - closing socket.");
                        self->socket.close();
                      }
                    });
}

void PeersConnection::initialize() {
  if (isInitialized())
    return;

  if (peer_addresses.size() == 0)
    return;

  if (local_peer_idx < 0 || local_peer_idx >= peer_addresses.size())
    return;

  auto self = std::static_pointer_cast<PeersConnection>(shared_from_this());
  asio::ip::tcp::resolver resolver(io_context);

  for (int idx = local_peer_idx + 1; idx < peer_addresses.size(); ++idx) {
    std::string address = peer_addresses[idx];
    GM_INF("PeersConnection", "Adding Beta Peer " << idx << " (" << address << ")");

    std::string host;
    std::string port;
    split_address_service(address, host, port);

    auto endpoints = resolver.resolve(host, port);
    if (endpoints.empty()) {
      GM_ERR("PeersConnection", "Cannot resolve " << host << ":" << port);
      throw std::invalid_argument("Cannot resolve peer address");
    }

    beta_peers.push_back(std::make_shared<Peer>(io_context, self, address, endpoints));
  }

  asio::ip::tcp::resolver::results_type bind_endpoints;
  {
    std::string address = peer_addresses[local_peer_idx];
    GM_INF("PeersConnection", "Bind address " << address);

    std::string host;
    std::string port;
    split_address_service(address, host, port);

    bind_endpoints = resolver.resolve(host, port);

    for (auto end : bind_endpoints)
      GM_INF("PeersConnection", "Resolved "
             << end.host_name()
             << " : "
             << end.service_name());
  }

  server_acceptor =
    std::make_shared<asio::ip::tcp::acceptor>
    (io_context, *bind_endpoints.begin());

  accept();

  for (auto peer : beta_peers) {

    auto ptr = std::make_shared<asio::deadline_timer>
      (io_context);
    ptr->expires_from_now(FIRST_CONNECT_DELAY_MS);

    ptr->async_wait([peer, ptr](const std::error_code&){
        peer->connect();
      });
  }

  io_thread = std::thread([this](){ this->runContext(); });

  Object::initialize();
}

void PeersConnection::accept() {

  std::weak_ptr<PeersConnection>
    weak_self(std::static_pointer_cast<PeersConnection>(shared_from_this()));

  GM_INF("PeersConnection", "Listening to incoming connections");
  server_acceptor->async_accept
    ([weak_self] (std::error_code ec, asio::ip::tcp::socket socket) {

      auto self = weak_self.lock();
      if (!self) return;

      if (ec)
        GM_WRN("PeersConnection", "Incoming connection problem (" << ec << ")");

      auto endpoint = socket.remote_endpoint();
      GM_INF("PeersConnection", "Received incoming connection (" << endpoint.address() << ":" << endpoint.port() << ")");

      self->alpha_peers.push_back(std::make_shared<Peer>(self->io_context,
                                                         self,
                                                         std::move(socket)));
      self->alpha_peers.back()->readHandshake();

      self->accept();
    });
}

void PeersConnection::waitForConnection() {
  GM_INF("PeersConnection", "Waiting for connection");
  while (true) {

    if (io_context.stopped()) return;

    std::size_t connect_count = 0;
    std::stringstream ss;

    ss << "a";
    for (auto peer : alpha_peers) {
      if (peer->isConnected())
        ++connect_count;
      ss << (peer->isConnected() ? "1" : "0");
    }

    ss << "b";
    for (auto peer : beta_peers) {
      if (peer->isConnected())
        ++connect_count;
      ss << (peer->isConnected() ? "1" : "0");
    }

    GM_VVINF("PeersConnection",
             "Connection wait status is " << ss.str()
             << " on " << getPeersCount());

    assert(connect_count <= getPeersCount() - 1);
    if (connect_count == getPeersCount() - 1)
      break;

    std::this_thread::yield();
  }
  GM_INF("PeersConnection", "All peers connected - done waiting");
}

void PeersConnection::runContext() {
  while (true) {
    try {
      GM_INF("PeersConnection", "Running ASIO io context");
      io_context.run();
      break;
    }
    catch (std::exception& e) {
      GM_WRN("PeersConnection", "Exception caught from ASIO io context: " << e.what());
    }
  }
  GM_INF("PeersConnection", "Done running ASIO io context");
}

void PeersConnection::addProtocol(std::shared_ptr<Protocol> p) {
  protocols.push_back(p);
}

void PeersConnection::removeProtocol(std::shared_ptr<Protocol> p) {
  std::remove_if(protocols.begin(),
                 protocols.end(),
                 [p](std::weak_ptr<Protocol> m) {
                   std::shared_ptr<Protocol> mp = m.lock();
                   // Remove also stall protocol pointers
                   return !mp || p == mp;
                 });
}

END_NAMESPACE_GMNETWORK;
