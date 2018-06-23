
#include <gmNetwork/Protocol.hh>

#include <gmNetwork/PeersConnection.hh>

BEGIN_NAMESPACE_GMNETWORK;

GM_OFI_DEFINE(Protocol);
GM_OFI_POINTER(Protocol, connection, PeersConnection, Protocol::setConnection);

void Protocol::setConnection(std::shared_ptr<PeersConnection> conn) {

  auto self = std::dynamic_pointer_cast<Protocol>(this->shared_from_this());

  if (connection) connection->removeProtocol(self);
  connection = conn;
  if (connection) connection->addProtocol(self);
}

std::shared_ptr<PeersConnection> Protocol::getConnection() {
  return connection;
}

void Protocol::close() {
  std::shared_ptr<PeersConnection> old_connection = connection;
  connection = nullptr;
  if (old_connection) old_connection->close();
  GM_INF("PeersConnection", "Protocol closed");
}

void Protocol::sendMessage(std::vector<char> data) {
  if (!connection)
    return;

  Message m = {
    -1,
    getProtocolFlag(),
    data
  };
  connection->sendMessage(m);
}

void Protocol::waitForConnection() {
  if (connection)
    connection->waitForConnection();
}

END_NAMESPACE_GMNETWORK;
