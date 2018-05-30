
#include <gmNetwork/NetSync.hh>

BEGIN_NAMESPACE_GMNETWORK;

GM_OFI_DEFINE(NetSync);
GM_OFI_PARAM(NetSync, peer, std::string, NetSync::addPeer);
GM_OFI_PARAM(NetSync, bind, std::string, NetSync::setBindAddress);

#define DEFAULT_SERVICE "20401"

NetSync::NetSync() {}

NetSync::~NetSync() {
  io_context.stop();
  io_thread.join();
}

void NetSync::addPeer(std::string address) {
  if (isInitialized())
    throw std::logic_error("Adding peer after initialization is not supported");

  std::string host;
  std::string port;
  split_address_service(address, host, port);

  asio::ip::tcp::resolver resolver(io_context);
  auto endpoints = resolver.resolve(host, port);
  if (endpoints.empty()) {
    GM_ERR("NetSync", "Cannot resolve " << host << ":" << port);
    throw std::invalid_argument("Cannot resolve peer address");
  }

  peers.push_back(std::make_shared<Peer>(io_context, address, endpoints));
}

void NetSync::setBindAddress(std::string address) {
  if (isInitialized())
    throw std::logic_error("Setting bind address after initialization is not supported");

  bind_address = address;
  std::string host;
  std::string port;
  split_address_service(address, host, port);

  asio::ip::tcp::resolver resolver(io_context);
  bind_endpoints = resolver.resolve(host, port);
  if (bind_endpoints.empty()) {
    GM_ERR("NetSync", "Cannot resolve " << host << ":" << port);
    throw std::invalid_argument("Cannot resolve bind address");
  }
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
                        } else {
                          GM_ERR("NetSync", "Could not connect: " << ec.message());
                        }
                      });
}

bool NetSync::Peer::match(asio::ip::tcp::endpoint ep) {
  GM_VINF("NetSync", "Comparing endpoint " << ep << " (" << this << ")");
  for (auto &pt : endpoints) {
    GM_VINF("NetSync", " with peer endpoint " << pt.host_name() << ":" << pt.service_name() << " (" << this << ")");
    if (pt == ep) return true;
  }
  return false;
}

void NetSync::initialize() {
  if (peers.size() == 0)
    return;

  if (bind_endpoints.empty())
    return;

  GM_INF("NetSync", "Bind address " << bind_address);
  for (auto end : bind_endpoints)
    GM_INF("NetSync", "Resolved "
           << end.host_name()
           << " : "
           << end.service_name());

  server_acceptor =
    std::make_shared<asio::ip::tcp::acceptor>
    (io_context, *bind_endpoints.begin());

  accept();

  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  for (auto peer : peers) {
    if (peer->address != bind_address)
      peer->connect();
  }

  io_thread = std::thread([this](){ this->runContext(); });
  Object::initialize();
}

void NetSync::accept() {
  GM_INF("NetSync", "Listening to incoming connections");
  server_acceptor->async_accept
    ([=] (std::error_code ec, asio::ip::tcp::socket socket) {

      if (ec)
        GM_WRN("NetSync", "Incoming connection problem (" << ec << ")");

      auto endpoint = socket.remote_endpoint();
      GM_INF("NetSync", "Received incoming connection (" << endpoint.address() << ":" << endpoint.port() << ")");

      bool found_match = false;
      endpoint.port(server_acceptor->local_endpoint().port());
      for (auto &peer : peers)
        if (peer->match(endpoint)) {
          if (found_match)
            GM_WRN("NetSync", "Incoming connection match multiple peers");
          peer->socket = std::move(socket);
          found_match = true;
        }
      if (!found_match)
        GM_WRN("NetSync", "No match among peers for incoming connection");

      bool all_connected = true;
      for (auto peer : peers)
        if (peer->isConnected() == false)
          all_connected = false;

      if (!all_connected)
        accept();
    });
}

void NetSync::waitForConnection() {
  bool wait = true;
  while (wait) {

    if (io_context.stopped()) return;

    wait = false;
    for (auto peer : peers)
      wait |= !peer->isConnected();
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
