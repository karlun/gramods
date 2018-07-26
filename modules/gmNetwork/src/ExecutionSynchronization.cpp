
#include <gmNetwork/ExecutionSynchronization.hh>

#include <gmNetwork/PeersConnection.hh>

BEGIN_NAMESPACE_GMNETWORK;

GM_OFI_DEFINE_SUB(ExecutionSynchronization, Protocol);

ExecutionSynchronization::ExecutionSynchronization()
  : waiting(false),
    closing(false) {}

ExecutionSynchronization::~ExecutionSynchronization() {
  close();
}

void ExecutionSynchronization::waitForAll() {
  std::shared_ptr<PeersConnection> _connection = getConnection();
  if (!_connection)
    return;

  GM_VINF("ExecutionSynchronization", "Waiting for all");

  std::vector<char> data;
  assert(_connection->getLocalPeerIdx() < 256);
  data.push_back((char)_connection->getLocalPeerIdx());

  GM_VINF("ExecutionSynchronization", "Notifying peers about waiting");
  sendMessage(data);

  {
    std::unique_lock<std::mutex> lck(waiting_lock);

    if (waiting_peers.size() == _connection->getPeersCount() - 1) {
      GM_VINF("ExecutionSynchronization", "Last waiting peer - no waiting");
      waiting_peers.clear();
      return;
    }

    waiting = true;
    waiting_condition.wait(lck, [this]{ return closing || !waiting; });
    GM_VINF("ExecutionSynchronization", "Done waiting");
  }
}

void ExecutionSynchronization::close() {

  std::unique_lock<std::mutex> lck(waiting_lock);
  if (closing) return;
  closing = true;

  if (waiting) {
    waiting = false;
    lck.unlock();
    waiting_condition.notify_all();
  } else {
    lck.unlock();
  }

  Protocol::close();
}

void ExecutionSynchronization::processMessage(Message m) {
  GM_VINF("ExecutionSynchronization", "Got message");

  std::shared_ptr<PeersConnection> _connection = connection;
  if (!_connection)
    return;

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
  GM_VINF("ExecutionSynchronization", "Waiting notification from peer " << (int)peer << " (got " << waiting_peers.size() << " of " << (_connection->getPeersCount() - 1) << ")");

  if (closing || !waiting)
    return;

  if (waiting_peers.size() == _connection->getPeersCount() - 1) {
    GM_VINF("ExecutionSynchronization", "Enough peers have notified " << (int)peer);
    waiting_peers.clear();
    waiting = false;
    lck.unlock();
    waiting_condition.notify_all();
  }
}

END_NAMESPACE_GMNETWORK;
