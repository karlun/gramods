
#ifndef GRAMODS_NETWORK_NETSYNC
#define GRAMODS_NETWORK_NETSYNC

#include <gmNetwork/config.hh>

#include <gmCore/Object.hh>
#include <gmCore/OFactory.hh>

#include <asio.hpp>
#include <map>
#include <vector>

BEGIN_NAMESPACE_GMNETWORK;

/**
   Execution synchronization over network, i.e. barrier.
*/
class NetSync
  : public gmCore::Object {

public:

  NetSync();
  ~NetSync();

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
     Connects to the peers.
  */
  void initialize();

  /**
     Waits until all peers have connected.
  */
  void waitForConnection();

  /**
     Waits until all peers have called this method. It is up to the
     client code to avoid deadlock or contention, and make sure that
     it is the same call that synchronizes.
   */
  void waitForAll();

  GM_OFI_DECLARE(NetSync);

private:

  struct Peer {

    Peer (asio::io_context &io_context,
          std::string address,
          asio::ip::tcp::resolver::results_type endpoints)
      : io_context(io_context),
        socket(io_context),
        address(address),
        endpoints(endpoints),
        is_connected(false) {}

    Peer (asio::io_context &io_context,
          asio::ip::tcp::socket socket)
      : io_context(io_context),
        socket(std::move(socket)),
        is_connected(false) {}

    bool connect();

    bool isConnected() { return is_connected; }

    void sendHandshake();
    void readHandshake();

  private:

    asio::io_context &io_context;
    bool is_connected;
    asio::ip::tcp::socket socket;
    char buffer_data[1024];

    std::string address;
    asio::ip::tcp::resolver::results_type endpoints;
  };

  void split_address_service(std::string comb, std::string &host, std::string &port);

  void runContext();
  void accept();

  asio::io_context io_context;
  std::thread io_thread;

  std::vector<std::shared_ptr<Peer>> alpha_peers;
  std::vector<std::shared_ptr<Peer>> beta_peers;
  std::vector<std::string> peer_addresses;
  int local_peer_idx;

  std::shared_ptr<asio::ip::tcp::acceptor> server_acceptor;
};

END_NAMESPACE_GMNETWORK;

#endif
