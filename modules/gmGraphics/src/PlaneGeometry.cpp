
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

  Eigen::Vector3f position;
  Eigen::Vector3f normal;
};

PlaneGeometry::PlaneGeometry()
  : Geometry(new Impl) {}

PlaneGeometry::~PlaneGeometry() {}

bool PlaneGeometry::getCameraFromPosition(Camera vfrustum,
                                          Eigen::Vector3f position,
                                          Camera &rfrustum) {
  return getCameraFromPosition(vfrustum, position, rfrustum);
}

bool PlaneGeometry::Impl::getCameraFromPosition(Camera vfrustum,
                                                Eigen::Vector3f position,
                                                Camera &rfrustum) {
  // TODO: calculate the render frustum
  return false;
}

std::string PlaneGeometry::getMapperCode() {
  return R"lang=glsl(
uniform vec3 pg_position;
uniform vec3 pg_normal;

vec3 getIntersection(vec3 pos, vec3 dir) {
  return pos + dir * (pg_normal * (pg_position - pos) / (dir * pg_normal));
}

bool isInside(vec3 pos) {
  return false;
}
)lang=glsl";
}

void PlaneGeometry::setMapperUniforms(GLuint program_id) {
  glUniform3fv(glGetUniformLocation(program_id, "pg_position"), 1, _impl->position.data());
  glUniform3fv(glGetUniformLocation(program_id, "pg_normal"), 1, _impl->normal.data());
}

void PlaneGeometry::setPosition(gmTypes::float3 p) {
  _impl->position = Eigen::Vector3f(p[0], p[1], p[2]);
}

void PlaneGeometry::setNormal(gmTypes::float3 n) {
  _impl->normal = Eigen::Vector3f(n[0], n[1], n[2]).normalized();
}

void PlaneGeometry::setQuaternion(gmTypes::float4 q) {
  Eigen::Quaternionf Q(q[0], q[1], q[2], q[3]);
  _impl->normal = Q * Eigen::Vector3f(0, 0, 1);
}

void PlaneGeometry::setAxisAngle(gmTypes::float4 aa) {
  Eigen::Quaternionf Q(Eigen::Quaternionf::AngleAxisType
                       (aa[3], Eigen::Vector3f(aa[0], aa[1], aa[2]).normalized()));
  _impl->normal = Q * Eigen::Vector3f(0, 0, 1);
}


END_NAMESPACE_GMGRAPHICS;
