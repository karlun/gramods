
#include <gmGraphics/PlaneGeometry.hh>
#include "Geometry.impl.hh"

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE_SUB(PlaneGeometry, Geometry);
GM_OFI_PARAM(PlaneGeometry, position, gmTypes::float3, PlaneGeometry::setPosition);
GM_OFI_PARAM(PlaneGeometry, normal, gmTypes::float3, PlaneGeometry::setNormal);
GM_OFI_PARAM(PlaneGeometry, quaternion, gmTypes::float4, PlaneGeometry::setQuaternion);
GM_OFI_PARAM(PlaneGeometry, axisAngle, gmTypes::float4, PlaneGeometry::setAxisAngle);

struct PlaneGeometry::Impl
  : Geometry::Impl {

  bool getCameraFromPosition(Camera vfrustum,
                             Eigen::Vector3f position,
                             Camera &rfrustum);

  Eigen::Vector3f getIntersection(Eigen::Vector3f pos, Eigen::Vector3f dir);

  Eigen::Vector3f position;
  Eigen::Vector3f normal;
};

PlaneGeometry::PlaneGeometry()
  : Geometry(new Impl) {}

PlaneGeometry::~PlaneGeometry() {}

bool PlaneGeometry::getCameraFromPosition(Camera vfrustum,
                                          Eigen::Vector3f position,
                                          Camera &rfrustum) {
  auto impl = static_cast<Impl*>(_impl.get());
  return impl->getCameraFromPosition(vfrustum, position, rfrustum);
}

bool PlaneGeometry::Impl::getCameraFromPosition(Camera vfrustum,
                                                Eigen::Vector3f position,
                                                Camera &rfrustum) {

  if ((position - this->position).dot(normal) <
      std::numeric_limits<float>::epsilon())
    return false;

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
  Eigen::Vector3f TL = getIntersection(vfrustum.getPosition(),
                                       orientation * Eigen::Vector3f(top, left, -1).normalized());
  Eigen::Vector3f BL = getIntersection(vfrustum.getPosition(),
                                       orientation * Eigen::Vector3f(bottom, left, -1).normalized());
  Eigen::Vector3f TR = getIntersection(vfrustum.getPosition(),
                                       orientation * Eigen::Vector3f(top, right, -1).normalized());
  Eigen::Vector3f BR = getIntersection(vfrustum.getPosition(),
                                       orientation * Eigen::Vector3f(bottom, right, -1).normalized());

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

  rfrustum.setPose(position, orientation);
  rfrustum.setPlanes(std::min(TL[0], BL[0]),
                     std::max(TR[0], BR[0]),
                     std::min(BL[1], BR[1]),
                     std::max(TL[1], TR[1]));

  float left2, right2, top2, bottom2;
  rfrustum.getPlanes(left2, right2, bottom2, top2);

  return true;
}

std::string PlaneGeometry::getMapperCode() {
  return R"lang=glsl(
uniform vec3 pg_position;
uniform vec3 pg_normal;

vec3 getIntersection(vec3 pos, vec3 dir) {
  float t = dot(pg_normal, (pg_position - pos)) / dot(dir, pg_normal);
  return pos + dir * t;
}

)lang=glsl";
}

void PlaneGeometry::setMapperUniforms(GLuint program_id) {
  auto impl = static_cast<Impl*>(_impl.get());
  glUniform3fv(glGetUniformLocation(program_id, "pg_position"), 1, impl->position.data());
  glUniform3fv(glGetUniformLocation(program_id, "pg_normal"), 1, impl->normal.data());
}

Eigen::Vector3f PlaneGeometry::Impl::getIntersection(Eigen::Vector3f pos, Eigen::Vector3f dir) {
  float t = normal.dot(position - pos) / dir.dot(normal);
  return pos + dir * t;
}

void PlaneGeometry::setPosition(gmTypes::float3 p) {
  auto impl = static_cast<Impl*>(_impl.get());
  impl->position = Eigen::Vector3f(p[0], p[1], p[2]);
}

void PlaneGeometry::setNormal(gmTypes::float3 n) {
  auto impl = static_cast<Impl*>(_impl.get());
  impl->normal = Eigen::Vector3f(n[0], n[1], n[2]).normalized();
}

void PlaneGeometry::setQuaternion(gmTypes::float4 q) {
  auto impl = static_cast<Impl*>(_impl.get());
  Eigen::Quaternionf Q(q[0], q[1], q[2], q[3]);
  impl->normal = Q * Eigen::Vector3f(0, 0, 1);
}

void PlaneGeometry::setAxisAngle(gmTypes::float4 aa) {
  Eigen::Quaternionf Q(Eigen::Quaternionf::AngleAxisType
                       (aa[3], Eigen::Vector3f(aa[0], aa[1], aa[2]).normalized()));
  auto impl = static_cast<Impl*>(_impl.get());
  impl->normal = Q * Eigen::Vector3f(0, 0, 1);
}


END_NAMESPACE_GMGRAPHICS;
