
#include <gmGraphics/IntersectionVisitor.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

void IntersectionVisitor::apply(gmCore::Object *obj) {
  if (auto *node = dynamic_cast<Node *>(obj)) {
    node_path.push_back(node);
    Node::TransformStackVisitor::apply(obj);

    auto line = this->line.getInSpace(stack.back());
    auto ratio_list = node->getIntersections(line);
    intersections.reserve(intersections.size() + ratio_list.size());
    for (auto r : ratio_list) {
      auto pos = line.getPosition(r);
      intersections.push_back({/*.local_position = */ pos,
                               /*.position = */ stack.back() * pos,
                               /*.node_path = */ node_path});
    }

    node_path.pop_back();
  } else {
    Node::TransformStackVisitor::apply(obj);
  }
}

END_NAMESPACE_GMGRAPHICS;
