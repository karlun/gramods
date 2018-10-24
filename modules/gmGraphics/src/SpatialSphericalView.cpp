
#include <gmGraphics/SpatialSphericalView.hh>

#include <gmGraphics/CubeMap.hh>
#include <gmGraphics/GLUtils.hh>

#include <GL/glew.h>
#include <GL/gl.h>

#include <vector>
#include <Eigen/Eigen>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE_SUB(SpatialSphericalView, StereoscopicView);
GM_OFI_PARAM(SpatialSphericalView, cubeMapResolution, int, SpatialSphericalView::setCubeMapResolution);
GM_OFI_PARAM(SpatialSphericalView, linearInterpolation, bool, SpatialSphericalView::setLinearInterpolation);
GM_OFI_PARAM(SpatialSphericalView, coverageAngle, float, SpatialSphericalView::setCoverageAngle);
GM_OFI_PARAM(SpatialSphericalView, projection, int, SpatialSphericalView::setProjection);
GM_OFI_PARAM(SpatialSphericalView, position, gmTypes::float3, SpatialSphericalView::setPosition);
GM_OFI_PARAM(SpatialSphericalView, radius, float, SpatialSphericalView::setRadius);
GM_OFI_PARAM(SpatialSphericalView, tiltAngle, float, SpatialSphericalView::setTiltAngle);

struct SpatialSphericalView::Impl {

  static const std::string fragment_code;
  static const std::string fisheye_mapper_code;
  static const std::string equirectangular_mapper_code;
  static const std::string mapper_pattern;

  void setProjection(int a);

  float coverage_angle = 2 * gramods_PI;
  bool make_square = false;
  Eigen::Vector3f position = Eigen::Vector3f::Zero();
  float radius = 10;
  float tilt_angle = 0;
  float eye_separation = 0;

  std::unique_ptr<CubeMap> cubemap;

  Impl() : cubemap(std::make_unique<CubeMap>()) {
    setProjection(0);
  }

  void renderFullPipeline(ViewSettings settings, Eye eye);

};

const std::string SpatialSphericalView::Impl::mapper_pattern = "MAPPER;";

const std::string SpatialSphericalView::Impl::fisheye_mapper_code = R"(
vec3 mapper(vec2 pos) {

  float r = sqrt(dot(pos.xy, pos.xy));
  if (r > 1) {
    return vec3(0, 0, 0);
  }

  float phi = 0.5 * r * min(coverageAngle, 6.28318530717958647693);
  float theta = atan(pos.y, pos.x);

  vec3 pix = vec3(cos(theta) * sin(phi), cos(phi), sin(theta) * sin(phi));

  return pix;
}
)";

const std::string SpatialSphericalView::Impl::equirectangular_mapper_code = R"(
vec3 mapper(vec2 pos) {

  float ay = pos.y * 1.57079632679489661923;
  float ax = pos.x * 0.5 * min(coverageAngle, 6.28318530717958647693);

  vec3 pix = vec3(cos(ay) * sin(ax), sin(ay), -cos(ay) * cos(ax));

  return pix;
}
)";

const std::string SpatialSphericalView::Impl::fragment_code = R"(
#version 330 core

uniform sampler2D texLeft;
uniform sampler2D texRight;
uniform sampler2D texBottom;
uniform sampler2D texTop;
uniform sampler2D texBack;
uniform sampler2D texFront;

uniform float coverageAngle;
uniform vec3 eye_position;
uniform float radius;
uniform float cubemap_radius;

in vec2 pos;
out vec4 fragColor;

MAPPER;

void colorFromTex(float x, float y, float z, sampler2D tex) {
  fragColor = vec4(texture(tex, 0.5 * vec2(x/z, y/z) + 0.5).rgb, 1);
}

vec3 tex_coord_direction(vec3 line) {
  vec3 pos_on_sphere = radius * normalize(line);
  return normalize(pos_on_sphere - eye_position);
}

float plane_line_d(vec3 line, vec3 n) {
  if (dot(line, n) <= 0) return 1e10;
  vec3 plane_position = cubemap_radius * n;
  float d = dot(plane_position - eye_position, n) / dot(line, n);
  return d;
}

void main() {

  vec3 pix_dir = mapper(pos);
  if (pix_dir.x == 0 && pix_dir.y == 0 && pix_dir.z == 0) {
    fragColor = vec4(0, 0, 0, 1);
    return;
  }

  vec3 tex_dir = tex_coord_direction(pix_dir);

  float d_left = plane_line_d(tex_dir, vec3(-1, 0, 0));
  float d_right = plane_line_d(tex_dir, vec3(1, 0, 0));
  float d_bottom = plane_line_d(tex_dir, vec3(0, -1, 0));
  float d_top = plane_line_d(tex_dir, vec3(0, 1, 0));
  float d_back = plane_line_d(tex_dir, vec3(0, 0, 1));
  float d_front = plane_line_d(tex_dir, vec3(0, 0, -1));

  float min_d = min(min(min(d_left, d_right), min(d_bottom, d_top)), min(d_back, d_front));
  vec3 pix = eye_position + min_d * tex_dir;

  if (min_d == d_left) {
    colorFromTex(-pix.z,  pix.y, -pix.x, texLeft);
    return;
  }

  if (min_d == d_right) {
    colorFromTex( pix.z,  pix.y,  pix.x, texRight);
    return;
  }

  if (min_d == d_bottom) {
    colorFromTex( pix.x, -pix.z, -pix.y, texBottom);
    return;
  }

  if (min_d == d_top) {
    colorFromTex( pix.x,  pix.z,  pix.y, texTop);
    return;
  }

  if (min_d == d_back) {
    colorFromTex(-pix.x,  pix.y,  pix.z, texBack);
    return;
  }

  if (min_d == d_front) {
    colorFromTex( pix.x,  pix.y, -pix.z, texFront);
    return;
  }

  fragColor = vec4(0.6, 0.3, 0.1, 1);
}
)";

SpatialSphericalView::SpatialSphericalView()
  : _impl(std::make_unique<Impl>()) {}

void SpatialSphericalView::renderFullPipeline(ViewSettings settings, Eye eye) {
  populateViewSettings(settings);
  _impl->eye_separation = eye_separation;
  _impl->renderFullPipeline(settings, eye);
}

void SpatialSphericalView::Impl::renderFullPipeline(ViewSettings settings, Eye eye) {

  Eigen::Vector3f pos = Eigen::Vector3f::Zero();
  Eigen::Quaternionf rot = Eigen::Quaternionf::Identity();
  Eigen::Quaternionf tilt
    (Eigen::AngleAxis<float>(tilt_angle, Eigen::Vector3f(-1, 0, 0)));

  if (settings.viewpoint) {
    pos = settings.viewpoint->getPosition();
    rot = settings.viewpoint->getOrientation();
  }

  switch (eye) {
  case Eye::LEFT:
    pos -= rot * Eigen::Vector3f(0.5f * eye_separation, 0.f, 0.f);
    break;
  case Eye::RIGHT:
    pos += rot * Eigen::Vector3f(0.5f * eye_separation, 0.f, 0.f);
  }

  GLint program_id = cubemap->getProgram();
  if (!program_id) {

    std::vector<std::shared_ptr<Renderer>> no_renderers;
    cubemap->renderFullPipeline(no_renderers, pos, tilt, make_square);
    program_id = cubemap->getProgram();

    if (!program_id) {
      static bool message_shown = false;
      if (!message_shown)
        GM_WRN("SpatialSphericalView", "Could not initialize cubemap");
      message_shown = true;
      return;
    }
  }

  Eigen::Vector3f offset = pos - position;

  glUseProgram(program_id);
  glUniform1f(glGetUniformLocation(program_id, "coverageAngle"), coverage_angle);
  glUniform3fv(glGetUniformLocation(program_id, "eye_position"), 1, offset.data());
  glUniform1f(glGetUniformLocation(program_id, "radius"), radius);
  glUniform1f(glGetUniformLocation(program_id, "cubemap_radius"), radius);
  glUseProgram(0);

  cubemap->setSpatialCubeMap(position, 2 * radius);
  cubemap->renderFullPipeline(settings.renderers, pos, tilt, make_square);
}

void SpatialSphericalView::Impl::setProjection(int a) {

  std::string fragment_code = this->fragment_code;
  assert(fragment_code.find(mapper_pattern) != std::string::npos);

  switch(a) {
  default:
    GM_WRN("PosedSphericalView", "Unavailable projection " << a);
  case 0:
    GM_VINF("PosedSphericalView", "Projection set to equirectangular");
    make_square = false;
    fragment_code.replace(fragment_code.find(mapper_pattern),
                          mapper_pattern.length(),
                          equirectangular_mapper_code);
    break;
  case 1:
    GM_VINF("PosedSphericalView", "Projection set to angular fisheye");
    make_square = false;
    fragment_code.replace(fragment_code.find(mapper_pattern),
                          mapper_pattern.length(),
                          fisheye_mapper_code);
    break;
  case 2:
    GM_VINF("PosedSphericalView", "Projection set to square angular fisheye");
    make_square = true;
    fragment_code.replace(fragment_code.find(mapper_pattern),
                          mapper_pattern.length(),
                          fisheye_mapper_code);
    break;
  }

  cubemap->setFragmentCode(fragment_code);
}

void SpatialSphericalView::setCubeMapResolution(int res) {
  _impl->cubemap->setCubeMapResolution(res);
}

void SpatialSphericalView::setLinearInterpolation(bool on) {
  _impl->cubemap->setLinearInterpolation(on);
}

void SpatialSphericalView::setCoverageAngle(float a) {
  _impl->coverage_angle = a;
}

void SpatialSphericalView::setProjection(int a) {
  _impl->setProjection(a);
}

void SpatialSphericalView::setPosition(gmTypes::float3 c) {
  assert(0);
  _impl->position = Eigen::Vector3f(c[0], c[1], c[2]);
}

void SpatialSphericalView::setRadius(float r) {
  _impl->radius = r;
}

void SpatialSphericalView::setTiltAngle(float a) {
  _impl->tilt_angle = a;
}

END_NAMESPACE_GMGRAPHICS;
