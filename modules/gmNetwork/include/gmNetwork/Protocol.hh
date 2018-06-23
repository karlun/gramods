
#ifndef GRAMODS_NETWORK_PROTOCOL
#define GRAMODS_NETWORK_PROTOCOL

#include <gmNetwork/config.hh>

#include <gmCore/Object.hh>
#include <gmCore/OFactory.hh>

BEGIN_NAMESPACE_GMNETWORK;

class PeersConnection;

/**
   The base of communication protocols. A protocol implementation will
   send data through a PeersConnnection instance, which will in turn
   forward incoming data to the designated protocol implementation.
*/
class Protocol
  : public gmCore::Object {

public:

  /**
     Data entity communicated by the connection to the designated
     protocol.
  */
  struct Message {

    /**
       The index of the peer from which this message originated.
    */
    int peer_idx;

    /**
       Flag indicating which protocol this message is encoded for.
    */
    char protocol;

    /**
       The complete set of data sent by the peer.
    */
    std::vector<char> data;
  };

public:

  /**
     Sets the connection to read messages from. This method will also
     register the protocol with the connection, for receiving messages
     with this protocols flag.
  */
  void setConnection(std::shared_ptr<PeersConnection> conn);

  /**
     Returns the connection that this protocol reads messages from.
  */
  std::shared_ptr<PeersConnection> getConnection();

  /**
     Closes the protocol and, if available, closes also its connection
     and sets it to nullptr. Call setConnection(nullptr) first to
     avoid closing the connection. This method is called by the
     connection if it is closed, directly or by another protocol.

     Sub classes should extend this method if necessary to react to
     closing the connection, but make sure to call Protocol::close to
     extend the default behaviour.
  */
  virtual void close();

  /**
     Called by the connection when data with this protocols flag has
     been received. The message is complete, as sent by the peer.

     It should be assumed that the call is made from a non-main
     thread. Also, the method should return promptly and leave heavy
     processing to either another, worker thread or to the client,
     main thread.
  */
  virtual void processMessage(Message m) {}

  /**
     Returns the byte sent in the message to indicate which protocol
     instance to call for interpretation and processing. Values 0-127
     are reserved for internal protocols while values 128-255 may be
     used by application specific protocols.
  */
  virtual char getProtocolFlag() { return 0; }

  /**
     Waits and does not return until all peers have connected to the
     connection associated with this protocol.
  */
  void waitForConnection();

  GM_OFI_DECLARE(Protocol);

protected:

  /**
     Convenience method for creating a message and sending this to all
     peers.
  */
  void sendMessage(std::vector<char> data);

  /**
     The connection that the protocol is communicating through.
  */
  std::shared_ptr<PeersConnection> connection;

};

END_NAMESPACE_GMNETWORK;

#endif
