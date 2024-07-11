
#ifndef GRAMODS_NETWORK_SYNCNODE
#define GRAMODS_NETWORK_SYNCNODE

#include <gmNetwork/config.hh>

// Required before gmCore/OFactory.hh for some compilers
#include <gmCore/io_size.hh>

#include <gmNetwork/Protocol.hh>

#include <gmCore/Object.hh>
#include <gmCore/OFactory.hh>
#include <gmCore/PreConditionViolation.hh>

#include <memory>
#include <set>

BEGIN_NAMESPACE_GMNETWORK;

/**
   A handler of network communication with multiple peers. This class
   will forward messages from protocol instances to all or selected
   peers, and forward incoming messages from the peers to the protocol
   instances.

   The SyncNode does not enforce a primary-replica or client-server
   architecture, but employ a peer-to-peer architecture.  However,
   client software may use the local peer index (getLocalPeerIdx) and
   assume, for example, that the peer with local peer index of zero is
   the primary.
*/
class SyncNode
  : public gmCore::Object {

public:

  SyncNode();
  ~SyncNode();

  /**
     Adds a peer to the list of peers in the cluster. This node's own
     address should also be added. It is assumed that this list is
     identical with all nodes in the cluster and that there is exactly
     one node with each local peer idx (0 - N-1).

     Valid syntax is ipv4 or ipv6 with or without port/service
     (defaults to 20401) e.g.

     -  nnn.nnn.nnn.nnn
     -  nnn.nnn.nnn.nnn:port
     -  nnnn:nnn::nnnn:nn:nnnn
     -  [nnnn:nnn:nnnn::nnn:nnn:nnnn:nnn]:port

     \gmXmlTag{gmNetwork,SyncNode,peer}
  */
  void addPeer(std::string address);

  /**
     Sets which of the peers that represents the local address.

     \gmXmlTag{gmNetwork,SyncNode,localPeerIdx}
  */
  void setLocalPeerIdx(size_t idx);

  /**
     Returns the index of the local peer.
  */
  size_t getLocalPeerIdx();

  /**
     Set whether or not the application should exit when connection to
     another node is lost. If set to true an ExitException is thrown
     from getConnectedPeers if a previously connected peer has lost
     its connection. Default is false, meaning that the node will
     *not* exit.
  */
  void setExitWhenAPeerIsDisconnected(bool on);

  /**
     Sets the number of seconds to wait before assuming that a silent
     peer has disconnected. If there has been no message after half
     this time, a keep-alive ping message is sent, that should produce
     a keep-alive pong message from the other peer. Default is a 1.0
     seconds timeout.
  */
  void setTimeoutDelay(float t);

  /**
     Returns the current timeout delay. See setTimeoutDelay for
     details.
  */
  float getTimeoutDelay();

  /**
     Wait until all listed peers have connected. This can safely be
     called more than once without side effects.
  */
  void waitForConnection();

  /**
     Check if all listed peers have connected.
  */
  bool isConnected();

  /**
     Return the number of peers, not including the local peer.
  */
  std::size_t getPeersCount();

  /**
     Return the indices of the currently connected peers.
  */
  std::set<std::size_t> getConnectedPeers();

  /**
     Returns an instance of the templated protocol. If there is no
     instance of this protocol type already, then a new instance is
     constructed and returned. Caller should not have to check for
     nullptr since any (rare) error should cause an exception to be
     thrown.

     Ownership of the protocol instance is kept by the sync node and
     all such instances will be deleted when the sync node is deleted,
     for example when going out of scope. Until then, however, the
     returned raw pointer is valid.
  */
  template<typename TYPE>
  TYPE * getProtocol() {
    static_assert(std::is_base_of<Protocol, TYPE>::value,
                  "Specified protocol is not a Protocol");

    std::string name = typeid(TYPE).name();
    Protocol * protocol = getProtocol(name);

    if (protocol) {
      TYPE * typed_protocol = dynamic_cast<TYPE*>(protocol);
      if (typed_protocol == nullptr)
        throw gmCore::PreConditionViolation
          ("Requested protocol exists but has wrong type.");
      return typed_protocol;
    }

    try {

      std::shared_ptr<TYPE> typed_protocol = std::make_shared<TYPE>();
      addProtocol(name, typed_protocol);

      return static_cast<TYPE*>(getProtocol(name));

    } catch (std::bad_weak_ptr &) {
      throw gmCore::PreConditionViolation
        ("SyncNode instance must be held by a std::shared_ptr");
    }
  }

  /**
     Returns the protocol instance associated with the specified name,
     or nullptr if there is no protocol with that name.
  */
  Protocol * getProtocol(std::string name);

  /**
     Returns the protocol instance associated with the specified
     protocol id, or nullptr if there is no protocol with that id.
  */
  Protocol * getProtocol(char id);

  /**
     Associates the specified protocol with the specified name. An
     exception will be thrown if there is already a protocol
     associated with that name.
  */
  void addProtocol(std::string name, std::shared_ptr<Protocol> prot);

  /**
     Sends the specified message to all peers.
  */
  void sendMessage(Protocol::Message mess);

  /**
     Called to initialize the Object. This should be called once only!
  */
  void initialize() override;

  /**
     Returns the default key, in Configuration, for the
     Object.
  */
  virtual std::string getDefaultKey() override { return "syncNode"; }

  GM_OFI_DECLARE;

private:

  struct Impl;
  std::unique_ptr<Impl> _impl;

};

END_NAMESPACE_GMNETWORK;

#endif
