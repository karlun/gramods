
#include <gmGraphics/SphereGeometry.hh>
#include "Geometry.impl.hh"

#include <gmCore/RunOnce.hh>
#include <gmCore/RunLimited.hh>

#include <limits>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE_SUB(SphereGeometry, Geometry);
GM_OFI_PARAM2(SphereGeometry, position, Eigen::Vector3f, setPosition);
GM_OFI_PARAM2(SphereGeometry, radius, float, setRadius);
GM_OFI_PARAM2(SphereGeometry, frustumSizeRatio, float, setFrustumSizeRatio);

struct SphereGeometry::Impl
  : Geometry::Impl {

  bool getCameraFromPosition(Camera vfrustum,
                             Eigen::Vector3f position,
                             Camera &rfrustum);

  bool getIntersection(Eigen::Vector3f pos,
                       Eigen::Vector3f dir,
                       Eigen::Vector3f &icp);

  Eigen::Vector3f position = Eigen::Vector3f::Zero();
  float radius = 10.f;
  float size_ratio = 1.f;
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

    float l, r, b, t;
    rfrustum.getClipPlanes(l, r, b, t);

    float l2 = (0.5f * l + 0.5f * r) + size_ratio * (l - (0.5f * l + 0.5f * r));
    float r2 = (0.5f * l + 0.5f * r) + size_ratio * (r - (0.5f * l + 0.5f * r));
    float b2 = (0.5f * t + 0.5f * b) + size_ratio * (b - (0.5f * t + 0.5f * b));
    float t2 = (0.5f * t + 0.5f * b) + size_ratio * (t - (0.5f * t + 0.5f * b));

    rfrustum.setClipPlanes(l2, r2, b2, t2);

    return true;
  }

  // Calculate the render frustum

  Eigen::Quaternionf orientation = vfrustum.getOrientation();

  float left, right, top, bottom;
  vfrustum.getClipPlanes(left, right, bottom, top);

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

  // Rotate view frustum orientation into projection (approximate
  // plane) to use as orientation for render frustum
  Eigen::Vector3f normalish = ((TR - BR).cross(BL - BR) +
                               (BR - BL).cross(TL - BL));
  orientation =
    Eigen::Quaternionf::FromTwoVectors(Eigen::Vector3f(0, 0, 1),
                                       normalish);
  Eigen::Vector3f upish = ((TL + TR) - (BL + BR));
  orientation =
    Eigen::Quaternionf::FromTwoVectors(orientation * Eigen::Vector3f(0, 1, 0),
    upish) * orientation;

  if (! have_TL || ! have_BL ||
      ! have_TR || ! have_BR) {
    GM_RUNLIMITED(GM_ERR("SphereGeometry", "Cannot estimate render frustum when view frustum corners do not intersect the geometry."), 1);
    return false;
  }

  // Corners of the view frustum in render frustum coordinates
  TL = orientation.conjugate() * (TL - position);
  BL = orientation.conjugate() * (BL - position);
  TR = orientation.conjugate() * (TR - position);
  BR = orientation.conjugate() * (BR - position);

  if (TL[2] > -std::numeric_limits<float>::epsilon() ||
      BL[2] > -std::numeric_limits<float>::epsilon() ||
      TR[2] > -std::numeric_limits<float>::epsilon() ||
      BR[2] > -std::numeric_limits<float>::epsilon()) {
    GM_RUNLIMITED(GM_ERR("SphereGeometry", "Cannot estimate render frustum when view frustum is (partially) behind the render position."), 1);
    return false;
  }

  // Normalize XY with Z to get planes at distance of 1
  TL *= -1 / TL[2];
  BL *= -1 / BL[2];
  TR *= -1 / TR[2];
  BR *= -1 / BR[2];

  // TODO: expand the render frustum to include also the circles of
  // sphere of the view frustum crop planes.

  float left2 = std::min(TL[0], BL[0]);
  float right2 = std::max(TR[0], BR[0]);
  float bottom2 = std::min(BL[1], BR[1]);
  float top2 = std::max(TL[1], TR[1]);

  // Until we have exact render frustum, that include also the circles
  // of sphere of the view frustum crop planes, we expand the frustum
  // by a fixed percentage.

  float left3 = (0.5f * left2 + 0.5f * right2) +
    size_ratio * (left2 - (0.5f * left2 + 0.5f * right2));
  float right3 = (0.5f * left2 + 0.5f * right2) +
    size_ratio * (right2 - (0.5f * left2 + 0.5f * right2));
  float bottom3 = (0.5f * top2 + 0.5f * bottom2) +
    size_ratio * (bottom2 - (0.5f * top2 + 0.5f * bottom2));
  float top3 = (0.5f * top2 + 0.5f * bottom2) +
    size_ratio * (top2 - (0.5f * top2 + 0.5f * bottom2));

  rfrustum.setPose(position, orientation);
  rfrustum.setClipPlanes(left3, right3, bottom3, top3);

  return true;
}

std::string SphereGeometry::getIntersectionCode() {
  return R"lang=glsl(
uniform vec3 sg_position;
uniform float sg_radius;
uniform bool sg_inside;

vec3 getIntersection(vec3 pos, vec3 dir) {
  float s = dot(dir, pos - sg_position) * dot(dir, pos - sg_position)
    - dot(pos - sg_position, pos - sg_position)
    + sg_radius * sg_radius;
  if (s < 0) return vec3(0, 0, 0);

  float t = sg_inside
    ? - dot(dir, pos - sg_position) + sqrt(s)
    : - dot(dir, pos - sg_position) - sqrt(s);
  if (t < 0) return vec3(0, 0, 0);

  return pos + dir * t;
}

)lang=glsl";
}

void SphereGeometry::setMapperUniforms(GLuint program_id) {
  auto impl = static_cast<Impl*>(_impl.get());
  glUniform3fv(glGetUniformLocation(program_id, "sg_position"), 1, impl->position.data());
  glUniform1f(glGetUniformLocation(program_id, "sg_radius"), impl->radius);
  glUniform1i(glGetUniformLocation(program_id, "sg_inside"), impl->inside);
}

bool SphereGeometry::Impl::getIntersection(Eigen::Vector3f pos,
                                           Eigen::Vector3f dir,
                                           Eigen::Vector3f &icp) {

  float s = dir.dot(pos - position) * dir.dot(pos - position)
    - (pos - position).dot(pos - position)
    + radius * radius;
  if (s < 0) return false;

  float t = inside
    ? - dir.dot(pos - position) + sqrtf(s)
    : - dir.dot(pos - position) - sqrtf(s);
  if (t < 0) return false;

  icp = pos + dir * t;
  return true;
}

void SphereGeometry::setPosition(Eigen::Vector3f p) {
  auto impl = static_cast<Impl*>(_impl.get());
  impl->position = p;
}

void SphereGeometry::setRadius(float r) {
  auto impl = static_cast<Impl*>(_impl.get());
  impl->radius = r;
}

void SphereGeometry::setFrustumSizeRatio(float r) {
  auto impl = static_cast<Impl*>(_impl.get());
  impl->size_ratio = r;
}


END_NAMESPACE_GMGRAPHICS;
