
#include <gmGraphics/Node.hh>

#include <gmGraphics/Renderer.hh>
#include <gmGraphics/Transform.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

void Node::TransformStackVisitor::apply(Object *node,
                                        const Eigen::Affine3f &transform) {
  stack.push_back(stack.back() * transform);
  apply(node);
  stack.pop_back();
}

void Node::NearFarVisitor::apply(gmCore::Object *node) {
  TransformStackVisitor::apply(node);

  if (auto *renderer = dynamic_cast<Renderer *>(node)) {

    float n = std::numeric_limits<float>::max();
    float f = std::numeric_limits<float>::min();

    renderer->getNearFar(camera, stack.back(), n, f);

    near = std::min(near, n);
    far = std::max(far, f);
  }
}

std::optional<std::pair<float, float>>
Node::NearFarVisitor::getNearFar() const {
  if (far <= near) return std::nullopt;
  if (far < std::numeric_limits<float>::epsilon()) return std::nullopt;

  // Expanding near-far span by 1% to accomodate for numerical errors
  // in transform processing, and using a minimum near plane of
  // 1:10000 to the far plan.
  float near =
      std::max(this->near - 0.01f * std::abs(this->near), float(1e-4) * far);
  float far = 1.01f * this->far;

  return std::pair<float, float>({near, far});
}

void Node::RenderVisitor::apply(gmCore::Object *node) {
  TransformStackVisitor::apply(node);

  if (auto *renderer = dynamic_cast<Renderer *>(node))
    renderer->render(camera, stack.back());
}

END_NAMESPACE_GMGRAPHICS;
