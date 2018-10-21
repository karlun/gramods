
#include <gmGraphics/AngularFisheyeView.hh>

#include <gmGraphics/CubeMap.hh>
#include <gmGraphics/GLUtils.hh>

#include <GL/glew.h>
#include <GL/gl.h>

#include <vector>
#include <Eigen/Eigen>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE_SUB(AngularFisheyeView, View);
GM_OFI_PARAM(AngularFisheyeView, cubeMapResolution, int, AngularFisheyeView::setCubeMapResolution);
GM_OFI_PARAM(AngularFisheyeView, linearInterpolation, bool, AngularFisheyeView::setLinearInterpolation);
GM_OFI_PARAM(AngularFisheyeView, coverageAngle, float, AngularFisheyeView::setCoverageAngle);

struct AngularFisheyeView::Impl {

  static const std::string fragment_code;
  float coverage_angle = gramods_PI;

  std::unique_ptr<CubeMap> cubemap;

  Impl() : cubemap(std::make_unique<CubeMap>()) {
    cubemap->setFragmentCode(fragment_code);
  }

};

const std::string AngularFisheyeView::Impl::fragment_code = R"(
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

void colorFromTex(float x, float y, float z, sampler2D tex) {
  fragColor = vec4(texture(tex, 0.5 * vec2(x/z, y/z) + 0.5).rgb, 1);
}

void main() {

  float r = sqrt(dot(pos.xy, pos.xy));
  if (r > 1) {
    fragColor = vec4(0, 0, 0, 1);
    return;
  }

  float phi = 0.5 * r * coverageAngle;
  float theta = atan(pos.y, pos.x);

  vec3 pix = vec3(cos(theta) * sin(phi), cos(phi), sin(theta) * sin(phi));

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
)";

AngularFisheyeView::AngularFisheyeView()
  : _impl(std::make_unique<Impl>()) {}

void AngularFisheyeView::renderFullPipeline(ViewSettings settings) {
  populateViewSettings(settings);

  GLint program_id = _impl->cubemap->getProgram();
  if (program_id) {
    glUseProgram(program_id);
    glUniform1f(glGetUniformLocation(program_id, "coverageAngle"), _impl->coverage_angle);
  }

  Eigen::Vector3f pos = Eigen::Vector3f::Zero();
  Eigen::Quaternionf rot = Eigen::Quaternionf::Identity();

  if (settings.viewpoint) {
    pos = settings.viewpoint->getPosition();
    rot = settings.viewpoint->getOrientation();
  }

  _impl->cubemap->renderFullPipeline(settings.renderers, pos, rot);
}

void AngularFisheyeView::setCubeMapResolution(int res) {
  _impl->cubemap->setCubeMapResolution(res);
}

void AngularFisheyeView::setLinearInterpolation(bool on) {
  _impl->cubemap->setLinearInterpolation(on);
}

void AngularFisheyeView::setCoverageAngle(float a) {
  _impl->coverage_angle = a;
}

END_NAMESPACE_GMGRAPHICS;
