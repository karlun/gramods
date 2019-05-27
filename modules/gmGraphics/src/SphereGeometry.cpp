
#include <gmGraphics/SphereGeometry.hh>
#include "Geometry.impl.hh"

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE_SUB(SphereGeometry, Geometry);
GM_OFI_PARAM(SphereGeometry, center, gmTypes::float3, SphereGeometry::setCenter);
GM_OFI_PARAM(SphereGeometry, radius, float, SphereGeometry::setRadius);

struct SphereGeometry::Impl
  : Geometry::Impl {

  bool getCameraFromPosition(Camera vfrustum,
                             Eigen::Vector3f position,
                             Camera &rfrustum);

  bool getIntersection(Eigen::Vector3f pos,
                       Eigen::Vector3f dir,
                       Eigen::Vector3f &icp);

  Eigen::Vector3f center;
  float radius;
};

SphereGeometry::SphereGeometry()
  : Geometry(new Impl) {}

SphereGeometry::~SphereGeometry() {}

bool SphereGeometry::getCameraFromPosition(Camera vfrustum,
                                           Eigen::Vector3f position,
                                           Camera &rfrustum) {
  auto impl = static_cast<Impl*>(_impl.get());
  return impl->getCameraFromPosition(vfrustum, position, rfrustum);
}

bool SphereGeometry::Impl::getCameraFromPosition(Camera vfrustum,
                                                 Eigen::Vector3f position,
                                                 Camera &rfrustum) {

  if ((position - vfrustum.getPosition()).norm() <
      std::numeric_limits<float>::epsilon()) {
    rfrustum = vfrustum;
    return true;
  }

  // Calculate the render frustum

  // Use view frustum orientation also for render frustum
  Eigen::Quaternionf orientation = vfrustum.getOrientation();

  float left, right, top, bottom;
  vfrustum.getPlanes(left, right, bottom, top);

  // Corners of the view frustum on the geometry in world coordinates
  Eigen::Vector3f TL, BL, TR, BR;
  bool have_TL = getIntersection
    (vfrustum.getPosition(),
     orientation * Eigen::Vector3f(left, top, -1).normalized(), TL);
  bool have_BL = getIntersection
    (vfrustum.getPosition(),
     orientation * Eigen::Vector3f(left, bottom, -1).normalized(), BL);
  bool have_TR = getIntersection
    (vfrustum.getPosition(),
     orientation * Eigen::Vector3f(right, top, -1).normalized(), TR);
  bool have_BR = getIntersection
    (vfrustum.getPosition(),
     orientation * Eigen::Vector3f(right, bottom, -1).normalized(), BR);

  // TODO: handle cases where a corner does not intersect the
  // geometry.

  if (! have_TL || ! have_BL ||
      ! have_TR || ! have_BR)
    return false;

  // Corners of the view frustum in render frustum coordinates
  TL = orientation.conjugate() * (TL - position);
  BL = orientation.conjugate() * (BL - position);
  TR = orientation.conjugate() * (TR - position);
  BR = orientation.conjugate() * (BR - position);

  // Normalize XY with Z to get planes at distance of 1
  TL *= -1 / TL[2];
  BL *= -1 / BL[2];
  TR *= -1 / TR[2];
  BR *= -1 / BR[2];

  // TODO: expand the render frustum to include also the circles of
  // sphere of the view frustum crop planes.

  rfrustum.setPose(position, orientation);
  rfrustum.setPlanes(std::min(TL[0], BL[0]),
                     std::max(TR[0], BR[0]),
                     std::min(BL[1], BR[1]),
                     std::max(TL[1], TR[1]));

  return true;
}

std::string SphereGeometry::getMapperCode() {
  return R"lang=glsl(
uniform vec3 sg_center;
uniform float sg_radius;
uniform bool sg_inside;

vec3 getIntersection(vec3 pos, vec3 dir) {
  float s = dot(dir, pos - center) * dot(dir, pos - center)
    - dot(pos - center, pos - center)
    + sg_radius * sg_radius;
  if (s < 0) return vec3(0, 0, 0);

  float t = inside
    ? - dot(dir, pos - center) - sqrt(s)
    : - dot(dir, pos - center) + sqrt(s);
  if (t < 0) return vec3(0, 0, 0);

  return pos + dir * t;
}

)lang=glsl";
}

void SphereGeometry::setMapperUniforms(GLuint program_id) {
  auto impl = static_cast<Impl*>(_impl.get());
  glUniform3fv(glGetUniformLocation(program_id, "sg_center"), 1, impl->center.data());
  glUniform3fv(glGetUniformLocation(program_id, "sg_radius"), 1, &impl->radius);
}

bool SphereGeometry::Impl::getIntersection(Eigen::Vector3f pos,
                                           Eigen::Vector3f dir,
                                           Eigen::Vector3f &icp) {

  float s = dir.dot(pos - center) * dir.dot(pos - center)
    - (pos - center).dot(pos - center)
    + radius * radius;
  if (s < 0) return false;

  float t = inside
    ? - dir.dot(pos - center) - sqrt(s)
    : - dir.dot(pos - center) + sqrt(s);
  if (t < 0) return false;

  icp = pos + dir * t;
  return true;
}

void SphereGeometry::setCenter(gmTypes::float3 c) {
  auto impl = static_cast<Impl*>(_impl.get());
  impl->center = Eigen::Vector3f(c[0], c[1], c[2]);
}

void SphereGeometry::setRadius(float r) {
  auto impl = static_cast<Impl*>(_impl.get());
  impl->radius = r;
}

END_NAMESPACE_GMGRAPHICS;
