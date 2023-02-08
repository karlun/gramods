
#ifndef GRAMODS_NETWORK_PROTOCOL
#define GRAMODS_NETWORK_PROTOCOL

#include <gmNetwork/config.hh>

#include <memory>
#include <mutex>
#include <vector>
#include <limits>
#include <set>

BEGIN_NAMESPACE_GMNETWORK;

class SyncNode;

/**
   TODO: write this
*/
class Protocol
  : public std::enable_shared_from_this<Protocol> {

public:

  static const size_t HEADER_LENGTH;

  Protocol() {}

  virtual ~Protocol() {}

  /**
     Returns the byte sent in the message to indicate which protocol
     instance to call for interpretation and processing. Values 0-127
     are reserved for internal protocols while values 128-255 may be
     used by application specific protocols.
  */
  virtual char getProtocolFlag() { return 0; }

  /**
     Data entity communicated by the connection to the designated
     protocol.
  */
  struct Message {

    /**
       Create an empty message.
    */
    Message() {}

    /**
       Create an incoming message initialized with the specified
       header data.
    */
    Message(std::vector<char> hdr);

    /**
       Create an outgoing message template with protocol id and data.
    */
    Message(char protocol, std::vector<char> data);

    /**
       Returns a vector filled with header data for this message. The
       length of the header is HEADER_LENGTH.
    */
    std::vector<char> getHeader();

    /**
       The index of the peer from which this message originated, or
       the maximum value of the type if not set.
    */
    size_t from_peer_idx = std::numeric_limits<size_t>::max();

    /**
       The index of the peer to which this message is sent, or the
       maximum value of the type if not set.
    */
    size_t to_peer_idx = std::numeric_limits<size_t>::max();

    /**
       Flag indicating which protocol this message is encoded for.
    */
    char protocol = 0;

    /**
       Expected length of the message as expressed in the message
       header. A complete message will satisfy the expression
       `data.length() == message_length`.
    */
    size_t length = 0;

    /**
       The complete set of data sent by the peer.
    */
    std::vector<char> data;
  };

  /**
     Called by the sync node when data with this protocols flag has
     been received. The message is complete, as sent by the peer.

     It should be assumed that the call is made from a non-main
     thread. Also, the method should return promptly and leave heavy
     processing to either another worker thread or to the main thread.
  */
  virtual void processMessage(Message m);

  /**
     Called by the sync node when connection to one of the peers has
     been broken.
  */
  virtual void lostPeer(size_t idx);

  void setSyncNode(SyncNode *sync_node);

protected:

  /**
     Convenience method for creating a message and sending this to all
     peers.
  */
  void sendMessage(std::vector<char> data);

  /**
     Convenience method for quering the SyncNode for the local peer idx.

     This will throw gmCore::PreConditionViolation exception if there
     is no SyncNode instance to get local peer idx from.
  */
  size_t getLocalPeerIdx();

  /**
     Convenience method for quering the SyncNode for the currently
     connected peers.
  */
  std::set<size_t> getConnectedPeers();

  /**
     The SyncNode instance this protocol communicates through or
     nullptr if it has gone out of scope. Use sync_node_lock to avoid
     race condition on this pointer.
  */
  SyncNode *sync_node;

  /**
     Lock for synchronizing the sync_node pointer.
  */
  std::mutex sync_node_lock;
};

END_NAMESPACE_GMNETWORK;

#endif
