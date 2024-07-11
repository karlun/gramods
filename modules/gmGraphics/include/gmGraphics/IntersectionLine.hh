
#ifndef GRAMODS_GRAPHICS_INTERSECTIONLINE
#define GRAMODS_GRAPHICS_INTERSECTIONLINE

#include <gmGraphics/config.hh>

#include <Eigen/Eigen>
#include <optional>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   Line for intersection with ray or segment.
*/
struct IntersectionLine {
  /**
     Creates a IntersectionLine for line segment intersection testing.
  */
  static IntersectionLine lineSegment(Eigen::Vector3f p0, Eigen::Vector3f p1);

  /**
     Creates a IntersectionLine that has infinite forward extent, for
     intersection testing.
  */
  static IntersectionLine forwardRay(Eigen::Vector3f p0, Eigen::Vector3f d);

  /**
     Creates a IntersectionLine for two-way infinite ray intersection testing.
  */
  static IntersectionLine infiniteRay(Eigen::Vector3f p0, Eigen::Vector3f d);

  /**
     Get this line but transformed into another coordinates base.
  */
  IntersectionLine getInSpace(const Eigen::Affine3f &M) const;

  /**
     Returns the position on the line that corresponds to a specified
     ratio.
  */
  inline Eigen::Vector3f getPosition(float ratio) { return p0 + dir * ratio; }

  const Eigen::Vector3f p0;         //< Segment start
  const Eigen::Vector3f dir;        //< Segment edge
  const Eigen::Vector3f dir_inv;    //< Inverted edge
  const std::optional<float> start; //< Start ratio on segment
  const std::optional<float> stop;  //< End ratio on segment
};

END_NAMESPACE_GMGRAPHICS;

#endif