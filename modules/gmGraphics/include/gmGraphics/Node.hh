
#ifndef GRAMODS_GRAPHICS_NODE
#define GRAMODS_GRAPHICS_NODE

#include <gmGraphics/config.hh>

#include <gmGraphics/Camera.hh>
#include <gmGraphics/IntersectionLine.hh>

#include <gmCore/Object.hh>

#include <Eigen/Eigen>
#include <vector>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   The scenegraph node base.
*/
class Node : public gmCore::Object {

public:

  typedef std::vector<std::shared_ptr<Node>> list;

  /**
     Base for Visitor:s that need to track the space transform through
     traversal.
  */
  struct TransformStackVisitor : Visitor {

    void apply(Object *node) override;

    std::vector<Eigen::Affine3f> stack {Eigen::Affine3f::Identity()};
  };

  /**
     Visitor that extracts the optimal near and far planes for a
     scenegraph.
  */
  struct NearFarVisitor : TransformStackVisitor {

    NearFarVisitor(Camera camera) : camera(camera) {}

    void apply(gmCore::Object *node) override;

    /**
       Makes positive, expands with epsilon and returns near and far
       values, iff the near and far values are reasonable.
    */
    std::optional<std::pair<float, float>> getNearFar() const;

    const Camera camera;
    float near = std::numeric_limits<float>::max();
    float far = std::numeric_limits<float>::min();
  };

  /**
     Visitor that renders a scenegraph to the currently active
     viewport.
  */
  struct RenderVisitor : TransformStackVisitor {

    RenderVisitor(Camera camera)
      : camera(camera) {}

    void apply(gmCore::Object *node) override;

    const Camera camera;
  };

  /**
     Check and return intersections between the provided line and the
     shape represented by this node.
  */
  virtual std::vector<float> getIntersections(const IntersectionLine &line) {
    return {};
  }

  /**
     Returns the default key, in Configuration, for the
     Object.
  */
  virtual std::string getDefaultKey() override { return "node"; }
};

END_NAMESPACE_GMGRAPHICS;

#endif
