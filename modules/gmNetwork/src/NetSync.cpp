
#include <gmNetwork/NetSync.hh>

BEGIN_NAMESPACE_GMNETWORK;

GM_OFI_DEFINE(NetSync);
GM_OFI_PARAM(NetSync, peer, std::string, NetSync::addPeer);
GM_OFI_PARAM(NetSync, localPeerIdx, int, NetSync::setLocalPeerIdx);

#define DEFAULT_SERVICE "20401"
#define HANDSHAKE_LENGTH 256

NetSync::NetSync() {}

NetSync::~NetSync() {
  io_context.stop();
  io_thread.join();
}

void NetSync::addPeer(std::string address) {
  if (isInitialized())
    throw std::logic_error("Adding peer after initialization is not supported");
  peer_addresses.push_back(address);
}

void NetSync::setLocalPeerIdx(int idx) {
  if (isInitialized())
    throw std::logic_error("Setting local peer after initialization is not supported");

  local_peer_idx = idx;
}

void NetSync::split_address_service
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

    GM_INF("NetSync", "IPv6 address '" << comb << "' = " << host << " : " << port);

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

    GM_INF("NetSync", "IPv4 address '" << comb << "' = " << host << " : " << port);
  }
}

bool NetSync::Peer::connect() {

  GM_VINF("NetSync", "Trying to connect");
  asio::async_connect(socket, endpoints,
                      [=](std::error_code ec, asio::ip::tcp::endpoint end) {
                        if (!ec) {
                          asio::ip::tcp::no_delay option(true);
                          socket.set_option(option);
                          is_connected = true;
                          GM_VINF("NetSync", "Connected");
                          sendHandshake();
                        } else {
                          GM_ERR("NetSync", "Could not connect: " << ec.message());
                          connect();
                        }
                      });
}

void NetSync::Peer::sendHandshake() {
  std::stringstream string_data;
  string_data << GRAMODS_NETSYNC_VERSION << " " << address;
  char data[HANDSHAKE_LENGTH] = { 0 };
  assert(string_data.str().size() < HANDSHAKE_LENGTH - 1);
  memcpy(data, string_data.str().c_str(),
         sizeof(string_data.str().c_str()) * string_data.str().size());

  asio::write(socket, asio::buffer(data, HANDSHAKE_LENGTH));
}

void NetSync::Peer::readHandshake() {
  asio::async_read(socket,
                   asio::buffer(buffer_data, HANDSHAKE_LENGTH),
                   [this](std::error_code ec, std::size_t length){
                     std::string NetSync_version;

                     std::stringstream data(buffer_data);
                     data >> NetSync_version >> address;
                     GM_INF("NetSync",
                            "Got handshake: " << length << " " << buffer_data
                            << " (version=" << NetSync_version
                            << ", address=" << address << ")");
                     is_connected = true;
                   });
}

void NetSync::initialize() {
  if (peer_addresses.size() == 0)
    return;

  if (local_peer_idx < 0 || local_peer_idx >= peer_addresses.size())
    return;

  asio::ip::tcp::resolver resolver(io_context);

  for (int idx = local_peer_idx + 1; idx < peer_addresses.size(); ++idx) {
    std::string address = peer_addresses[idx];
    GM_INF("NetSync", "Adding Beta Peer " << idx << " (" << address << ")");

    std::string host;
    std::string port;
    split_address_service(address, host, port);

    auto endpoints = resolver.resolve(host, port);
    if (endpoints.empty()) {
      GM_ERR("NetSync", "Cannot resolve " << host << ":" << port);
      throw std::invalid_argument("Cannot resolve peer address");
    }

    beta_peers.push_back(std::make_shared<Peer>(io_context, address, endpoints));
  }

  asio::ip::tcp::resolver::results_type bind_endpoints;
  {
    std::string address = peer_addresses[local_peer_idx];
    GM_INF("NetSync", "Bind address " << address);

    std::string host;
    std::string port;
    split_address_service(address, host, port);

    bind_endpoints = resolver.resolve(host, port);

    for (auto end : bind_endpoints)
      GM_INF("NetSync", "Resolved "
             << end.host_name()
             << " : "
             << end.service_name());
  }

  server_acceptor =
    std::make_shared<asio::ip::tcp::acceptor>
    (io_context, *bind_endpoints.begin());

  accept();

  io_thread = std::thread([this](){ this->runContext(); });

  for (auto peer : beta_peers)
    peer->connect();

  Object::initialize();
}

void NetSync::accept() {
  GM_INF("NetSync", "Listening to incoming connections");
  server_acceptor->async_accept
    ([=] (std::error_code ec, asio::ip::tcp::socket socket) {

      if (ec)
        GM_WRN("NetSync", "Incoming connection problem (" << ec << ")");

      auto endpoint = socket.remote_endpoint();
      GM_INF("NetSync", this << " received incoming connection (" << endpoint.address() << ":" << endpoint.port() << ")");

      alpha_peers.push_back(std::make_shared<Peer>(io_context, std::move(socket)));
      alpha_peers.back()->readHandshake();

      accept();
    });
}

void NetSync::waitForConnection() {
  while (true) {

    if (io_context.stopped()) return;

    bool wait = false;
    for (auto peer : alpha_peers)
      wait |= !peer->isConnected();
    for (auto peer : beta_peers)
      wait |= !peer->isConnected();

    if (wait == false)
      break;

    std::this_thread::yield();
  }
}

void NetSync::waitForAll() {
  if (io_context.stopped())
    return;
}

void NetSync::runContext() {
  while (true) {
    try {
      GM_INF("NetSync", "Running ASIO io context");
      io_context.run();
      break;
    }
    catch (std::exception& e) {
      GM_WRN("NetSync", "Exception caught from ASIO io context: " << e.what());
    }
  }
  GM_INF("NetSync", "Done running ASIO io context");
}

END_NAMESPACE_GMNETWORK;
