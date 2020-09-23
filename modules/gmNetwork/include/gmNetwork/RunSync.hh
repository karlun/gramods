
#ifndef GRAMODS_NETWORK_RUNSYNC
#define GRAMODS_NETWORK_RUNSYNC

#include <gmNetwork/Protocol.hh>

#include <set>
#include <mutex>
#include <condition_variable>

BEGIN_NAMESPACE_GMNETWORK;

/**
   Execution synchronization over network, i.e. barrier.
*/
class RunSync
  : public Protocol {

public:

  RunSync();
  ~RunSync();

  /**
     Waits until all peers have called this method. It is up to the
     client code to avoid deadlock or contention, and make sure that
     it is the same call that synchronizes.
   */
  void wait();

  /**
     Counts the number of peers that have called waitForAll and
     releases the waiting thread when all peers have notified this.
  */
  void processMessage(Message m) override;

  /**
     Called by the sync node when connection to one of the peers has
     been broken.
  */
  void lostPeer(size_t idx) override;

  /**
     Returns the header byte associated with the protocol, sent in the
     header of messages to indicate which protocol instance to call
     for interpretation and processing.
  */
  char getProtocolFlag() override { return 10; }

private:

  struct Impl;
  std::unique_ptr<Impl> _impl;
};

END_NAMESPACE_GMNETWORK;

#endif
