
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

void Group::traverse(Visitor *visitor) {
  for (auto &n : _impl->nodes) n->accept(visitor);
}

END_NAMESPACE_GMGRAPHICS;
