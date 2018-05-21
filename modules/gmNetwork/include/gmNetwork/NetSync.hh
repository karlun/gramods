
#ifndef GRAMODS_NETWORK_NETSYNC
#define GRAMODS_NETWORK_NETSYNC

#include <gmNetwork/config.hh>

#include <gmCore/Object.hh>
#include <gmCore/OFactory.hh>

BEGIN_NAMESPACE_GMNETWORK;

class NetSync
  : public gmCore::Object {

public:

  /**
     Adds a peer to connect to. This node's own address may be added.
  */
  void addPeer(std::string address);

  /**
     Sets which of the peer addresses to bind to.
  */
  void setBindAddress(std::string address);

  /**
     Bind the specified address and connects to the added peers.
  */
  void initialize();

  /**
     Waits until all peers have called this method. It is up to the
     client code to avoid deadlock or contention, and make sure that
     it is the same call that synchronizes.
   */
  void waitForAll(bool onlyConnected = false);

  GM_OFI_DECLARE(NetSync);

};

END_NAMESPACE_GMNETWORK;

#endif
