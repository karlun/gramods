
#include <gmNetwork/Protocol.hh>

#include <gmNetwork/SyncNode.hh>

BEGIN_NAMESPACE_GMNETWORK;

const size_t Protocol::HEADER_LENGTH = Message().getHeader().size();

Protocol::Message::Message(std::vector<uint8_t> hdr)
    : protocol(hdr[0]), length((uint32_t(hdr[1]) << 24) | //
                               (uint32_t(hdr[2]) << 16) | //
                               (uint32_t(hdr[3]) << 8) |  //
                               (uint32_t(hdr[4]) << 0)) {}

Protocol::Message::Message(uint8_t protocol, std::vector<uint8_t> data)
    : protocol(protocol), length(data.size()), data(data) {}

std::vector<uint8_t> Protocol::Message::getHeader() {
  if (length & 0xff000000) std::cerr << length << std::endl;
  return {protocol,
          (uint8_t)((length >> 24) & 0xff), //
          (uint8_t)((length >> 16) & 0xff), //
          (uint8_t)((length >> 8) & 0xff),  //
          (uint8_t)(length & 0xff)};
}

void Protocol::processMessage(Message) {}

void Protocol::lostPeer(size_t) {}

void Protocol::setSyncNode(SyncNode *sync_node) {
  std::lock_guard<std::mutex> guard(sync_node_lock);
  this->sync_node = sync_node;
}

void Protocol::sendMessage(std::vector<uint8_t> data) {
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
