
#include <gmNetwork/Protocol.hh>

#include <gmNetwork/SyncNode.hh>

BEGIN_NAMESPACE_GMNETWORK;

const size_t Protocol::HEADER_LENGTH = Message().getHeader().size();

Protocol::Message::Message(std::vector<char> hdr)
  : protocol(hdr[0]),
    length((hdr[1] << 24) +
           (hdr[2] << 16) +
           (hdr[3] <<  8) +
           (hdr[4] <<  0)) {}

Protocol::Message::Message(char protocol, std::vector<char> data)
  : protocol(protocol),
    length(data.size()),
    data(data) {}

std::vector<char> Protocol::Message::getHeader() {
  return { protocol,
           (char)((length >> 24) & 0xff),
           (char)((length >> 16) & 0xff),
           (char)((length >>  8) & 0xff),
           (char)((length      ) & 0xff) };
}

void Protocol::processMessage(Message) {}

void Protocol::lostPeer(size_t) {}

void Protocol::setSyncNode(SyncNode *sync_node) {
  std::lock_guard<std::mutex> guard(sync_node_lock);
  this->sync_node = sync_node;
}

void Protocol::sendMessage(std::vector<char> data) {
  std::lock_guard<std::mutex> guard(sync_node_lock);
  if (!sync_node) return;
  sync_node->sendMessage(Message(getProtocolFlag(), data));
}

size_t Protocol::getLocalPeerIdx() {
  std::lock_guard<std::mutex> guard(sync_node_lock);
  if (!sync_node) throw gmCore::PreConditionViolation("Protocol has no live SyncNode");
  return sync_node->getLocalPeerIdx();
}

std::set<size_t> Protocol::getConnectedPeers() {
  std::lock_guard<std::mutex> guard(sync_node_lock);
  if (!sync_node) return {};
  return sync_node->getConnectedPeers();
}

END_NAMESPACE_GMNETWORK;
