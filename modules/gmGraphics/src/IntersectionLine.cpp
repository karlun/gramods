
#include <gmGraphics/IntersectionVisitor.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

IntersectionLine IntersectionLine::lineSegment(Eigen::Vector3f p0,
                                               Eigen::Vector3f p1) {
  Eigen::Vector3f n = p1 - p0;
  double length = n.norm();
  n = n.normalized();
  return {p0, n, n.cwiseInverse(), 0, length};
}

IntersectionLine IntersectionLine::forwardRay(Eigen::Vector3f p0,
                                              Eigen::Vector3f d) {
  double length = d.norm();
  d = d.normalized();
  return {p0, d, d.cwiseInverse(), 0, std::nullopt};
}

IntersectionLine IntersectionLine::infiniteRay(Eigen::Vector3f p0,
                                               Eigen::Vector3f d) {
  double length = d.norm();
  d = d.normalized();
  return {p0, d, d.cwiseInverse(), std::nullopt, std::nullopt};
}

IntersectionLine IntersectionLine::getInSpace(const Eigen::Affine3f &M4) const {

  const auto M4_inv = M4.inverse();
  const auto &M3_inv = M4_inv.matrix().block<3, 3>(0, 0);

  Eigen::Vector3f new_dir = M3_inv * dir;
  float dir_length = new_dir.norm();
  new_dir /= dir_length;

  return {/*.p0 = */ M4_inv * p0,
          /*.dir = */ new_dir,
          /*.dir_inv = */ new_dir.cwiseInverse(),
          /*.start = */ start ? *start * dir_length : start,
          /*.stop = */ stop ? *stop * dir_length : stop};
}

END_NAMESPACE_GMGRAPHICS;
