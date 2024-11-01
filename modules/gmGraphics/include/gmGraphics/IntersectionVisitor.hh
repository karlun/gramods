
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
     Data associated with an intersection.
  */
  struct Intersection {
    /**
       The position of the intersection in the local space of the node
       where the intersection was detected.
    */
    const Eigen::Vector3f local_position;

    /**
       The position of the intersection in the space where the line
       was defined and the visitor was first applied to.
    */
    const Eigen::Vector3f position;

    /**
       The path taken through the scene graph to find this
       intersection. The front will point at the first node the
       visitor was applied to and the back will point at the node that
       detected the intersection.
    */
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
