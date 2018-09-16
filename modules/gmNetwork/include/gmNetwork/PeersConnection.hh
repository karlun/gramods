
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

   The PeersConnection does not enforce a master-slave or
   client-server architecture, but employ a peer-to-peer architecture.
   However, client software may use the local peer index
   (getLocalPeerIdx) and assume, for example, that the peer with local
   peer index of zero is the master.

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
  */
  void addPeer(std::string address);

  /**
     Sets which of the peers that represents the local address.
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

  GM_OFI_DECLARE;

private:

  /**
     A connection and communication handler for one single peer.
  */
  struct Peer
    : std::enable_shared_from_this<Peer> {

    Peer (asio::io_context &io_context,
          std::shared_ptr<PeersConnection> parent,
          std::string address,
          asio::ip::tcp::resolver::results_type endpoints)
      : io_context(io_context),
        parent(parent),
        socket(io_context),
        address(address),
        endpoints(endpoints),
        is_connected(false) {}

    Peer (asio::io_context &io_context,
          std::shared_ptr<PeersConnection> parent,
          asio::ip::tcp::socket socket)
      : io_context(io_context),
        parent(parent),
        socket(std::move(socket)),
        is_connected(false) {}

    bool connect();

    bool isConnected();

    void sendMessage(Protocol::Message mess);
    void readData();

    void sendHandshake();
    void readHandshake();

  private:

    asio::io_context &io_context;
    std::weak_ptr<PeersConnection> parent;
    asio::ip::tcp::socket socket;
    bool is_connected;
    std::mutex lock;

    std::unique_ptr<Protocol::Message> message;
    std::size_t message_length;

    std::string address;
    asio::ip::tcp::resolver::results_type endpoints;
  };

  void split_address_service(std::string comb, std::string &host, std::string &port);

  void runContext();
  void accept();

  static void routeMessage(std::shared_ptr<PeersConnection> self,
                           Protocol::Message mess);

  asio::io_context io_context;
  std::thread io_thread;
  bool closing;
  std::mutex system_lock;

  std::vector<std::shared_ptr<Peer>> alpha_peers;
  std::vector<std::shared_ptr<Peer>> beta_peers;
  std::vector<std::string> peer_addresses;
  int local_peer_idx;
  std::mutex peers_lock;

  std::shared_ptr<asio::ip::tcp::acceptor> server_acceptor;

  std::vector<std::weak_ptr<Protocol>> protocols;
  std::mutex protocols_lock;

  friend Peer;
};

END_NAMESPACE_GMNETWORK;

#endif
