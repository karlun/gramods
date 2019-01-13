
#include <gmNetwork/Protocol.hh>

#include <gmNetwork/PeersConnection.hh>

BEGIN_NAMESPACE_GMNETWORK;

GM_OFI_DEFINE(Protocol);
GM_OFI_POINTER(Protocol, connection, PeersConnection, Protocol::setConnection);

Protocol::~Protocol() {
  close();
}

void Protocol::setConnection(std::shared_ptr<PeersConnection> conn) {

  std::lock_guard<std::mutex> guard(connection_lock);

  auto self = std::dynamic_pointer_cast<Protocol>(this->shared_from_this());

  if (connection) connection->removeProtocol(self);
  connection = conn;
  if (connection) connection->addProtocol(self);
}

std::shared_ptr<PeersConnection> Protocol::getConnection() {
  std::lock_guard<std::mutex> guard(connection_lock);
  return connection;
}

void Protocol::close() {
  std::lock_guard<std::mutex> guard(connection_lock);
  std::shared_ptr<PeersConnection> old_connection = connection;
  connection = nullptr;
  if (old_connection) old_connection->close();
  GM_INF("PeersConnection", "Protocol closed");
}

void Protocol::sendMessage(std::vector<char> data) {
  std::lock_guard<std::mutex> guard(connection_lock);

  std::shared_ptr<PeersConnection> _connection = connection;
  if (!_connection)
    return;

  Message m = {
    -1,
    getProtocolFlag(),
    data
  };
  _connection->sendMessage(m);
}

void Protocol::waitForConnection() {
  std::lock_guard<std::mutex> guard(connection_lock);
  std::shared_ptr<PeersConnection> _connection = connection;
  if (connection)
    _connection->waitForConnection();
}

END_NAMESPACE_GMNETWORK;
