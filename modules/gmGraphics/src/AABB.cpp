
#include <gmGraphics/AABB.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

bool AABB::isIntersecting(const Line &line) {
  // Adapted from https://tavianator.com/2011/ray_box.html
  // https://en.wikipedia.org/wiki/Slab_method

  const double tx1 = (bmin.x() - line.p0.x()) * line.dir_inv.x();
  const double tx2 = (bmax.x() - line.p0.x()) * line.dir_inv.x();

  double tmin = std::min(tx1, tx2);
  double tmax = std::max(tx1, tx2);

  const double ty1 = (bmin.y() - line.p0.y()) * line.dir_inv.y();
  const double ty2 = (bmax.y() - line.p0.y()) * line.dir_inv.y();

  tmin = std::max(tmin, std::min(ty1, ty2));
  tmax = std::min(tmax, std::max(ty1, ty2));

  const double tz1 = (bmin.z() - line.p0.z()) * line.dir_inv.z();
  const double tz2 = (bmax.z() - line.p0.z()) * line.dir_inv.z();

  tmin = std::max(tmin, std::min(tz1, tz2));
  tmax = std::min(tmax, std::max(tz1, tz2));

  if (tmin > tmax) return false;

  return (!line.start || tmax > line.start) && (!line.stop || tmin < line.stop);
}

std::vector<float> AABB::getIntersections(const Line &line) {
  // Adapted from https://tavianator.com/2011/ray_box.html
  // https://en.wikipedia.org/wiki/Slab_method

  const double tx1 = (bmin.x() - line.p0.x()) * line.dir_inv.x();
  const double tx2 = (bmax.x() - line.p0.x()) * line.dir_inv.x();

  double tmin = std::min(tx1, tx2);
  double tmax = std::max(tx1, tx2);

  const double ty1 = (bmin.y() - line.p0.y()) * line.dir_inv.y();
  const double ty2 = (bmax.y() - line.p0.y()) * line.dir_inv.y();

  tmin = std::max(tmin, std::min(ty1, ty2));
  tmax = std::min(tmax, std::max(ty1, ty2));

  const double tz1 = (bmin.z() - line.p0.z()) * line.dir_inv.z();
  const double tz2 = (bmax.z() - line.p0.z()) * line.dir_inv.z();

  tmin = std::max(tmin, std::min(tz1, tz2));
  tmax = std::min(tmax, std::max(tz1, tz2));

  if (tmin > tmax) return {};

  std::vector<float> res;
  res.reserve(2);

  if ((!line.start || tmin > line.start) && (!line.stop || tmin < line.stop))
    res.push_back(tmin);

  if ((!line.start || tmax > line.start) && (!line.stop || tmax < line.stop))
    res.push_back(tmax);

  return res;
}

END_NAMESPACE_GMGRAPHICS;
