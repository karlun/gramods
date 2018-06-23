
#include <gmNetwork/ExecutionSynchronization.hh>

#include <gmNetwork/PeersConnection.hh>

BEGIN_NAMESPACE_GMNETWORK;

GM_OFI_DEFINE_SUB(ExecutionSynchronization, Protocol);

void ExecutionSynchronization::waitForAll() {
  if (!connection)
    return;

  std::unique_lock<std::mutex> lck(waiting_lock);

  std::vector<char> data;
  assert(connection->getLocalPeerIdx() < 256);
  data.push_back((char)connection->getLocalPeerIdx());

  sendMessage(data);

  if (waiting_peers.size() == connection->getPeersCount() - 1) {
    waiting_peers.clear();
    return;
  }

  waiting = true;
  while (waiting) waiting_condition.wait(lck);
}

void ExecutionSynchronization::close() {
  std::unique_lock<std::mutex> lck(waiting_lock);
  waiting = false;
  Protocol::close();
  waiting_condition.notify_all();
}

void ExecutionSynchronization::processMessage(Message m) {

  assert(m.data.size() == 1);
  if (m.data.size() != 1) {
    GM_ERR("ExecutionSynchronization", "Corrupt message");
    return;
  }

  char peer = m.data[0];

  std::unique_lock<std::mutex> lck(waiting_lock);

  if (waiting_peers.count(peer) != 0) {
    GM_ERR("ExecutionSynchronization", "Peer notifies multiple times that it is waiting.");
    return;
  }

  waiting_peers.insert(peer);

  if (!waiting)
    return;

  if (waiting_peers.size() == connection->getPeersCount() - 1) {
    waiting_peers.clear();
    waiting = false;
    waiting_condition.notify_all();
  }
}

END_NAMESPACE_GMNETWORK;
