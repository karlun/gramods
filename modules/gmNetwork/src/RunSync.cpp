
#include <gmNetwork/RunSync.hh>

#include <gmNetwork/SyncNode.hh>
#include <gmCore/Console.hh>

BEGIN_NAMESPACE_GMNETWORK;

struct RunSync::Impl {

  void wait(RunSync * run_sync);
  void processMessage(size_t local_peer_idx,
                      Message m);

  std::mutex impl_lock;
  std::condition_variable waiting_condition;
  std::set<size_t> waiting_peers_odd;
  std::set<size_t> waiting_peers_even;
  bool waiting_frame_odd = false;
  bool cancel_waiting = false;
};

namespace {
  bool is_waiting_for_peers(std::set<size_t> waiting_peers,
                            std::set<size_t> actual_peers) {

    if (actual_peers.empty()) return false;

    std::vector<size_t> peers_left;
    peers_left.reserve(actual_peers.size());

    std::set_difference(actual_peers.begin(), actual_peers.end(),
                        waiting_peers.begin(), waiting_peers.end(),
                        std::inserter(peers_left, peers_left.begin()));

    return ! peers_left.empty();
  }
}

RunSync::RunSync()
  : _impl(std::make_unique<Impl>()) {}

RunSync::~RunSync() {
  std::unique_lock<std::mutex> guard(_impl->impl_lock);
  _impl->cancel_waiting = true;
  _impl->waiting_condition.notify_all();
}

void RunSync::wait() {
  _impl->wait(this);
}

void RunSync::Impl::wait(RunSync * run_sync) {

  size_t local_peer_idx = run_sync->getLocalPeerIdx();

  std::unique_lock<std::mutex> guard(impl_lock);

  if (local_peer_idx > 255)
    throw gmCore::PreConditionViolation("Support only for up to 255 peers.");

  GM_DBG2("RunSync", local_peer_idx << " Initializing waiting; notifying all peers about waiting in frame " << waiting_frame_odd << ".");

  guard.unlock();
  run_sync->sendMessage({ (char)local_peer_idx,
                          waiting_frame_odd ? (char)1 : (char)0 });
  guard.lock();

  auto & waiting_peers = waiting_frame_odd ?
    waiting_peers_odd : waiting_peers_even;

  while (is_waiting_for_peers(waiting_peers,
                              run_sync->getConnectedPeers()) &&
         !cancel_waiting) {
    waiting_condition.wait_for(guard,
                               // Check every 200 ms if a peer is lost
                               std::chrono::milliseconds(200));
  }

  cancel_waiting = false;
  waiting_frame_odd = ! waiting_frame_odd;
  waiting_peers.clear();

  GM_DBG2("RunSync", local_peer_idx << " Done waiting (" << (waiting_frame_odd ? 1 : 0) << ").");
}

void RunSync::processMessage(Message m) {
  _impl->processMessage(getLocalPeerIdx(), m);
}

void RunSync::Impl::processMessage(size_t local_peer_idx,
                                   Message m) {

  assert(m.data.size() == 2);
  if (m.data.size() != 2) {
    GM_ERR("RunSync", local_peer_idx << " Corrupt message");
    return;
  }

  char peer_idx = m.data[0];
  char frame_odd = m.data[1];

  GM_DBG2("RunSync", local_peer_idx << " Got message of size " << m.data.size()
          << " (" << (int)peer_idx << ", " << (int)frame_odd << ")");

  std::unique_lock<std::mutex> guard(impl_lock);

  std::set<size_t> & waiting_peers = frame_odd ? waiting_peers_odd : waiting_peers_even;

  if (waiting_peers.count(peer_idx) != 0) {
    GM_WRN("RunSync", local_peer_idx << " Peer " << int(peer_idx) << " notifies multiple times that it is waiting in frame " << (int)frame_odd << ".");
    return;
  }

  waiting_peers.insert(peer_idx);
  GM_DBG2("RunSync", local_peer_idx << " Waiting notification from " << (int)peer_idx << " (got " << waiting_peers.size() << ")");

  if (waiting_frame_odd == bool(frame_odd))
    waiting_condition.notify_all();
}

void RunSync::lostPeer(size_t) {
  std::lock_guard<std::mutex> guard(_impl->impl_lock);
  _impl->waiting_condition.notify_all();
}

END_NAMESPACE_GMNETWORK;
