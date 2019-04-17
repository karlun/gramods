
#include <gmGraphics/PosedSphericalView.hh>

#include <gmGraphics/CubeMap.hh>
#include <gmGraphics/GLUtils.hh>

#include <GL/glew.h>
#include <GL/gl.h>

#include <vector>
#include <Eigen/Eigen>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE_SUB(PosedSphericalView, View);
GM_OFI_PARAM(PosedSphericalView, cubeMapResolution, int, PosedSphericalView::setCubeMapResolution);
GM_OFI_PARAM(PosedSphericalView, linearInterpolation, bool, PosedSphericalView::setLinearInterpolation);
GM_OFI_PARAM(PosedSphericalView, coverageAngle, float, PosedSphericalView::setCoverageAngle);
GM_OFI_PARAM(PosedSphericalView, projectionType, int, PosedSphericalView::setProjectionType);

struct PosedSphericalView::Impl {

  static const std::string fragment_code;
  static const std::string fisheye_mapper_code;
  static const std::string equirectangular_mapper_code;
  static const std::string mapper_pattern;

  void setProjectionType(int a);

  float coverage_angle = 2 * gramods_PI;
  bool make_square = false;

  std::unique_ptr<CubeMap> cubemap;

  Impl() : cubemap(std::make_unique<CubeMap>()) {
    setProjectionType(0);
  }

};

const std::string PosedSphericalView::Impl::mapper_pattern = "MAPPER;";

const std::string PosedSphericalView::Impl::fisheye_mapper_code = R"lang=glsl(
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
)lang=glsl";

const std::string PosedSphericalView::Impl::equirectangular_mapper_code = R"lang=glsl(
vec3 mapper(vec2 pos) {

  float ay = pos.y * 1.57079632679489661923;
  float ax = pos.x * 0.5 * min(coverageAngle, 6.28318530717958647693);

  vec3 pix = vec3(cos(ay) * sin(ax), sin(ay), -cos(ay) * cos(ax));

  return pix;
}
)lang=glsl";

const std::string PosedSphericalView::Impl::fragment_code = R"lang=glsl(
#version 330 core

uniform sampler2D texLeft;
uniform sampler2D texRight;
uniform sampler2D texBottom;
uniform sampler2D texTop;
uniform sampler2D texBack;
uniform sampler2D texFront;

uniform float coverageAngle;

in vec2 pos;
out vec4 fragColor;

MAPPER;

void colorFromTex(float x, float y, float z, sampler2D tex) {
  fragColor = texture(tex, 0.5 * vec2(x/z, y/z) + 0.5);
}

void main() {

  vec3 pix = mapper(pos);
  if (pix.x == 0 && pix.y == 0 && pix.z == 0) {
    fragColor = vec4(0, 0, 0, 1);
    return;
  }

  if (pix.x < -abs(pix.y) && pix.x < -abs(pix.z)) {
    colorFromTex(-pix.z,  pix.y, -pix.x, texLeft);
    return;
  }

  if (pix.x >=  abs(pix.y) && pix.x >=  abs(pix.z)) {
    colorFromTex( pix.z,  pix.y,  pix.x, texRight);
    return;
  }

  if (pix.y <= -abs(pix.x) && pix.y <= -abs(pix.z)) {
    colorFromTex( pix.x, -pix.z, -pix.y, texBottom);
    return;
  }

  if (pix.y >=  abs(pix.x) && pix.y >=  abs(pix.z)) {
    colorFromTex( pix.x,  pix.z,  pix.y, texTop);
    return;
  }

  if (pix.z >=  abs(pix.x) && pix.z >=  abs(pix.y)) {
    colorFromTex(-pix.x,  pix.y,  pix.z, texBack);
    return;
  }

  if (pix.z <= -abs(pix.x) && pix.z <= -abs(pix.y)) {
    colorFromTex( pix.x,  pix.y, -pix.z, texFront);
    return;
  }

  fragColor = vec4(0.6, 0.3, 0.1, 1);
}
)lang=glsl";

PosedSphericalView::PosedSphericalView()
  : _impl(std::make_unique<Impl>()) {}

void PosedSphericalView::renderFullPipeline(ViewSettings settings) {
  populateViewSettings(settings);

  GLint program_id = _impl->cubemap->getProgram();
  if (program_id) {
    glUseProgram(program_id);
    glUniform1f(glGetUniformLocation(program_id, "coverageAngle"), _impl->coverage_angle);
  }

  Eigen::Vector3f eye_pos = Eigen::Vector3f::Zero();
  Eigen::Quaternionf head_rot = Eigen::Quaternionf::Identity();

  if (settings.viewpoint) {
    eye_pos = settings.viewpoint->getPosition();
    head_rot = settings.viewpoint->getOrientation();
  }

  _impl->cubemap->renderFullPipeline(settings.renderers, eye_pos, head_rot, _impl->make_square);
}

void PosedSphericalView::Impl::setProjectionType(int a) {

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
    GM_VINF("PosedSphericalView", "Projection set to angular fisheye");
    make_square = true;
    fragment_code.replace(fragment_code.find(mapper_pattern),
                          mapper_pattern.length(),
                          fisheye_mapper_code);
    break;
  }

  cubemap->setFragmentCode(fragment_code);
}

void PosedSphericalView::setCubeMapResolution(int res) {
  _impl->cubemap->setCubeMapResolution(res);
}

void PosedSphericalView::setLinearInterpolation(bool on) {
  _impl->cubemap->setLinearInterpolation(on);
}

void PosedSphericalView::setCoverageAngle(float a) {
  _impl->coverage_angle = a;
}

void PosedSphericalView::setProjectionType(int a) {
  _impl->setProjectionType(a);
}

END_NAMESPACE_GMGRAPHICS;
