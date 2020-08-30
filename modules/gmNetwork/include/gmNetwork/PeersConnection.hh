
#ifndef GRAMODS_NETWORK_PEERSCONNECTION
#define GRAMODS_NETWORK_PEERSCONNECTION

#include <gmNetwork/config.hh>

#include <gmNetwork/Protocol.hh>

#include <gmCore/Object.hh>
#include <gmCore/OFactory.hh>

#include <asio.hpp>

#include <map>
#include <vector>
#include <deque>
#include <mutex>

BEGIN_NAMESPACE_GMNETWORK;

/**
   A handler of network communication with multiple peers. This class
   will forward messages from protocol instances to all or selected
   peers, and forward incoming messages from the peers to the protocol
   instances.

   The PeersConnection does not enforce any hierarchical relationship
   between the nodes, but employ a peer-to-peer architecture.
   However, client software may use the local peer index
   (getLocalPeerIdx) and assume, for example, that the peer with local
   peer index of zero is the primary.

   The connection object will only hold weak pointers to the protocol
   instances, so the client code retain shared pointers to keep them
   alive.
*/
class PeersConnection
  : public gmCore::Object {

public:

  PeersConnection();
  ~PeersConnection();

  /**
     Closes all connections and disables the network abilities. Call
     to release waiting calls, such as waitForConnection.
  */
  void close();

  /**
     Adds a peer to connect to. This node's own address may be added.

     Valid syntax is ipv4 or ipv6 with or without port/service
     (defaults to 20401) e.g.

     -  nnn.nnn.nnn.nnn
     -  nnn.nnn.nnn.nnn:port
     -  nnnn:nnn::nnnn:nn:nnnn
     -  [nnnn:nnn:nnnn::nnn:nnn:nnnn:nnn]:port

     \b XML-attribute: \c peer
  */
  void addPeer(std::string address);

  /**
     Sets which of the peers that represents the local address.

     \b XML-attribute: \c localPeerIdx
  */
  void setLocalPeerIdx(int idx);

  /**
     Returns the index of the local peer.
  */
  int getLocalPeerIdx();

  /**
     Returns the number of peers in the cloud. This count includes the
     local peer.
  */
  std::size_t getPeersCount();

  /**
     Connects to the peers.
  */
  void initialize();

  /**
     Adds the specific Protocol implementation to listen to messages
     in this PeersConnection.
  */
  void addProtocol(std::shared_ptr<Protocol> p);

  /**
     Removes the specific Protocol implementation to listen to
     messages in this PeersConnection. Does nothing if p is not
     in the list.
  */
  void removeProtocol(std::shared_ptr<Protocol> p);

  /**
     Waits and does not return until all peers have connected.
  */
  void waitForConnection();

  void sendMessage(Protocol::Message mess);

  /**
     Returns the default key, in Configuration, for the
     Object.
  */
  virtual std::string getDefaultKey() { return "connection"; }

  GM_OFI_DECLARE;

private:

  struct Impl;
  std::shared_ptr<Impl> _impl;

};

END_NAMESPACE_GMNETWORK;

#endif
