
#ifndef GRAMODS_NETWORK_EXECUTIONSYNCHRONIZATION
#define GRAMODS_NETWORK_EXECUTIONSYNCHRONIZATION

#include <gmNetwork/Protocol.hh>

#include <set>
#include <mutex>
#include <condition_variable>

BEGIN_NAMESPACE_GMNETWORK;

/**
   Execution synchronization over network, i.e. barrier.
*/
class ExecutionSynchronization
  : public Protocol {

public:

  ExecutionSynchronization();
  ~ExecutionSynchronization();

  /**
     Waits until all peers have called this method. It is up to the
     client code to avoid deadlock or contention, and make sure that
     it is the same call that synchronizes.
   */
  void waitForAll();

  /**
     Closes the protocol and releases waiting threads.
  */
  void close();

  /**
     Counts the number of peers that have called waitForAll and
     releases the waiting thread when all peers have notified this.
  */
  void processMessage(Message m);

  /**
     Returns the header byte associated with the protocol, sent in the
     header of messages to indicate which protocol instance to call
     for interpretation and processing.
  */
  char getProtocolFlag() { return 10; }

  GM_OFI_DECLARE;

private:

  bool waiting;
  std::mutex waiting_lock;
  std::condition_variable waiting_condition;
  std::set<char> waiting_peers;

  bool closing;
};

END_NAMESPACE_GMNETWORK;

#endif
