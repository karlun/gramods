
#ifndef GRAMODS_GRAPHICS_TRANSFORM
#define GRAMODS_GRAPHICS_TRANSFORM

#include <gmGraphics/Group.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   The scenegraph transform base.
*/
class Transform : public Group {

public:
  virtual Eigen::Affine3f getTransform() = 0;

  void accept(Visitor *visitor) override {
    if (auto *ts_visitor =
            dynamic_cast<Node::TransformStackVisitor *>(visitor)) {
      ts_visitor->apply(this, getTransform());
    } else
      visitor->apply(this);
  }
};

END_NAMESPACE_GMGRAPHICS;

#endif
