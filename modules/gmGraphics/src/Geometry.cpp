
#include "Geometry.impl.hh"

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE(Geometry);
GM_OFI_PARAM(Geometry, inside, bool, Geometry::setInside);

Geometry::Geometry(Impl * impl)
  : _impl(impl ? impl : new Impl) {}

Geometry::~Geometry() {}

void Geometry::setInside(bool on) {
  _impl->inside = on;
}

END_NAMESPACE_GMGRAPHICS;
