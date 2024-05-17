
#include <gmGraphics/Node.hh>

#include <gmGraphics/Renderer.hh>
#include <gmGraphics/Transform.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

void Node::TransformStackVisitor::apply(Object *node) {
  if (auto *transform = dynamic_cast<Transform *>(node)) {
    stack.push_back(stack.back() * transform->getTransform());
    Object::Visitor::apply(node);
    stack.pop_back();
  } else {
    Object::Visitor::apply(node);
  }
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

  float near = this->near;

  if (near < std::numeric_limits<float>::epsilon())
    near = std::numeric_limits<float>::epsilon() * (1e3 * far);

  float epsilon = (far - near) * std::numeric_limits<float>::epsilon();

  return std::pair<float, float>({near - epsilon, far + epsilon});
}

void Node::RenderVisitor::apply(gmCore::Object *node) {
  TransformStackVisitor::apply(node);

  if (auto *renderer = dynamic_cast<Renderer *>(node))
    renderer->render(camera, stack.back());
}

END_NAMESPACE_GMGRAPHICS;
