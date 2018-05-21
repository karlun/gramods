
#ifndef GRAMODS_NETWORK_PEERTOPEERCONNECTION
#define GRAMODS_NETWORK_PEERTOPEERCONNECTION

#include <gmNetwork/Connection.hh>

BEGIN_NAMESPACE_GMNETWORK;

/**
   Peer-to-peer connection
*/
class PeerToPeerConnection
  : public Connection {

public:

  void setPort(int port);
  int getPort();

  void addPeer(std::string host);
  void removePeer(std::string host);
  void removeAllPeers();

  bool send(std::vector<char> data);
  bool receive(std::vector<char> &data);

  Status getStatus();

private:
};

END_NAMESPACE_GMNETWORK;

#endif
