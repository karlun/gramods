
#ifndef GRAMODS_GRAPHICS_INTERSECTIONVISITOR
#define GRAMODS_GRAPHICS_INTERSECTIONVISITOR

#include <gmGraphics/config.hh>

#include <gmGraphics/IntersectionLine.hh>
#include <gmGraphics/Node.hh>

#include <Eigen/Eigen>
#include <vector>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   Visitor that collects intersections.
*/
struct IntersectionVisitor : Node::TransformStackVisitor {

  /**
     Intersection
  */
  struct Intersection {
    const float ratio;
    const Eigen::Vector3f local_position;
    const Eigen::Vector3f position;
    const std::vector<gmGraphics::Node *> node_path;
  };

  IntersectionVisitor(IntersectionLine line) : line(line) {}

  void apply(gmCore::Object *node) override;

  const IntersectionLine line;
  std::vector<gmGraphics::Node *> node_path;
  std::vector<Intersection> intersections;
};

END_NAMESPACE_GMGRAPHICS;

#endif
