
#ifndef GRAMODS_NETWORK_NETSYNC
#define GRAMODS_NETWORK_NETSYNC

#include <gmNetwork/config.hh>

#include <gmCore/Object.hh>
#include <gmCore/OFactory.hh>

#include <asio.hpp>
#include <set>

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
     Sets which of the peer addresses to bind to.

     Valid syntax is ipv4 or ipv6 with or without port/service
     (defaults to 20401) e.g.

     -  nnn.nnn.nnn.nnn
     -  nnn.nnn.nnn.nnn:port
     -  nnnn:nnn::nnnn:nn:nnnn
     -  [nnnn:nnn:nnnn::nnn:nnn:nnnn:nnn]:port
  */
  void setBindAddress(std::string address);

  /**
     Bind the specified address and connects to the added peers.
  */
  void initialize();

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

    bool connect();

    bool match(asio::ip::tcp::endpoint ep);

    bool isConnected() { return is_connected; }

    //private:

    asio::io_context &io_context;
    asio::ip::tcp::socket socket;
    std::string address;
    asio::ip::tcp::resolver::results_type endpoints;
    bool is_connected;
  };

  void split_address_service(std::string comb, std::string &host, std::string &port);

  void runContext();
  void accept();

  asio::io_context io_context;
  std::thread io_thread;

  std::vector<std::shared_ptr<Peer>> peers;

  std::shared_ptr<asio::ip::tcp::acceptor> server_acceptor;
  std::string bind_address;
  asio::ip::tcp::resolver::results_type bind_endpoints;
};

END_NAMESPACE_GMNETWORK;

#endif
