
#include "Geometry.impl.hh"

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE(Geometry);
GM_OFI_PARAM2(Geometry, inside, bool, setInside);

Geometry::Geometry(Impl * impl)
  : _impl(impl ? impl : new Impl) {}

Geometry::~Geometry() {}

bool Geometry::getCameraFromPosition(Camera,
                                     Eigen::Vector3f,
                                     Camera &) {
  return false;
}

void Geometry::setInside(bool on) {
  _impl->inside = on;
}

END_NAMESPACE_GMGRAPHICS;
