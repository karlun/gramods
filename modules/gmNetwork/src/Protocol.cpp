
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

void Protocol::sendMessage(std::vector<char> data) {
  //std::lock_guard<std::mutex> guard(connection_lock);

  std::shared_ptr<SyncNode> sync_node = this->sync_node.lock();
  if (!sync_node) return;

  sync_node->sendMessage(Message(getProtocolFlag(), data));
}

END_NAMESPACE_GMNETWORK;
