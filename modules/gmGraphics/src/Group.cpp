
#include <gmGraphics/Group.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE(Group);
GM_OFI_POINTER2(Group, node, gmGraphics::Node, addNode);

struct Group::Impl {
  std::vector<std::shared_ptr<Node>> nodes;
};

Group::Group()
  : _impl(std::make_unique<Impl>()) {}

Group::~Group() {}

void Group::addNode(std::shared_ptr<Node> node) {
  _impl->nodes.push_back(node);
}

void Group::removeNode(std::shared_ptr<Node> node) {
  _impl->nodes.erase(
      std::remove(_impl->nodes.begin(), _impl->nodes.end(), node),
      _impl->nodes.end());
}

void Group::removeNode(size_t idx) {
  if (idx >= _impl->nodes.size())
    throw gmCore::InvalidArgument(GM_STR(
        "Specified index " << idx << " is out of bounds ("
                           << _impl->nodes.size() << " items available)"));
  _impl->nodes.erase(_impl->nodes.begin() + idx);
}

std::vector<std::shared_ptr<Node>> Group::getNodes() {
  return _impl->nodes;
}

void Group::traverse(Visitor *visitor) {
  for (auto &n : _impl->nodes) n->accept(visitor);
}

END_NAMESPACE_GMGRAPHICS;
