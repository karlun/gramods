
#ifndef GRAMODS_GRAPHICS_AABB
#define GRAMODS_GRAPHICS_AABB

#include <gmGraphics/config.hh>
#include <gmGraphics/IntersectionVisitor.hh>

#include <Eigen/Eigen>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   Axis aligned bounding box values with operations.
*/
class AABB {

public:
  AABB(const AABB &bb) : bmin(bb.bmin), bmax(bb.bmax) {}
  AABB(const Eigen::Vector3f &p) : bmin(p), bmax(p) {}
  AABB(const AABB &bb, const Eigen::Vector3f &p)
    : bmin(bb.bmin.cwiseMin(p)), bmax(bb.bmax.cwiseMax(p)) {}
  AABB(const AABB &op1, const AABB &op2)
    : bmin(op1.bmin.cwiseMin(op2.bmin)), bmax(op1.bmax.cwiseMax(op2.bmax)) {}

  AABB operator+(const Eigen::Vector3f &op) { return AABB(*this, op); }
  AABB operator+(const AABB &op) { return AABB(*this, op); }
  AABB &operator+=(const Eigen::Vector3f &op) { return *this = *this + op; }
  AABB &operator+=(const AABB &op) { return *this = *this + op; }

  const Eigen::Vector3f &min() const { return bmin; }
  const Eigen::Vector3f &max() const { return bmax; }

  Eigen::Vector3f getCenter() { return 0.5f * (bmax + bmin); }

  AABB &addOffset(const Eigen::Vector3f &op) {
    bmax += op;
    bmin += op;
    return *this;
  }

  std::vector<Eigen::Vector3f> getCorners() const {
    return {
        Eigen::Vector3f(bmin.x(), bmin.y(), bmin.z()),
        Eigen::Vector3f(bmin.x(), bmin.y(), bmax.z()),
        Eigen::Vector3f(bmin.x(), bmax.y(), bmin.z()),
        Eigen::Vector3f(bmin.x(), bmax.y(), bmax.z()),
        Eigen::Vector3f(bmax.x(), bmin.y(), bmin.z()),
        Eigen::Vector3f(bmax.x(), bmin.y(), bmax.z()),
        Eigen::Vector3f(bmax.x(), bmax.y(), bmin.z()),
        Eigen::Vector3f(bmax.x(), bmax.y(), bmax.z()),
    };
  }

  /**
     Line for intersection checking.
  */
  typedef IntersectionLine Line;

  /**
     Check if the specified line has an intersection with the AABB.
  */
  bool isIntersecting(const Line &line);

  /**
     Retrieves the intersections for the specified line as ratio on
     the provided line. Use Line::getPosition to convert ratio into
     position.
  */
  std::vector<float> getIntersections(const Line &line);

private:
  Eigen::Vector3f bmin;
  Eigen::Vector3f bmax;
};

END_NAMESPACE_GMGRAPHICS;

#endif
