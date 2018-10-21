
#include <gmGraphics/EquirectangularView.hh>

#include <gmGraphics/CubeMap.hh>
#include <gmGraphics/GLUtils.hh>

#include <GL/glew.h>
#include <GL/gl.h>

#include <vector>
#include <Eigen/Eigen>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE_SUB(EquirectangularView, View);
GM_OFI_PARAM(EquirectangularView, cubeMapResolution, int, EquirectangularView::setCubeMapResolution);
GM_OFI_PARAM(EquirectangularView, linearInterpolation, bool, EquirectangularView::setLinearInterpolation);

struct EquirectangularView::Impl {

  static const std::string fragment_code;

  std::unique_ptr<CubeMap> cubemap;

  Impl() : cubemap(std::make_unique<CubeMap>()) {
    cubemap->setFragmentCode(fragment_code);
  }

};

const std::string EquirectangularView::Impl::fragment_code = R"(
#version 330 core

uniform sampler2D texLeft;
uniform sampler2D texRight;
uniform sampler2D texBottom;
uniform sampler2D texTop;
uniform sampler2D texBack;
uniform sampler2D texFront;

in vec2 pos;
out vec4 fragColor;

void colorFromTex(float x, float y, float z, sampler2D tex) {
  fragColor = vec4(texture(tex, 0.5 * vec2(x/z, y/z) + 0.5).rgb, 1);
}

void main() {
  
  float ay = pos.y * 1.57079632679489661923;
  float ax = pos.x * 3.14159265358979323846;

  vec3 pix = vec3(cos(ay) * sin(ax), sin(ay), -cos(ay) * cos(ax));

  if (pix.x < -abs(pix.y) && pix.x < -abs(pix.z)) {
    colorFromTex(-pix.z,  pix.y, -pix.x, texLeft);
    return;
  }

  if (pix.x >  abs(pix.y) && pix.x >  abs(pix.z)) {
    colorFromTex( pix.z,  pix.y,  pix.x, texRight);
    return;
  }

  if (pix.y < -abs(pix.x) && pix.y < -abs(pix.z)) {
    colorFromTex( pix.x, -pix.z, -pix.y, texBottom);
    return;
  }

  if (pix.y >  abs(pix.x) && pix.y >  abs(pix.z)) {
    colorFromTex( pix.x,  pix.z,  pix.y, texTop);
    return;
  }

  if (pix.z >  abs(pix.x) && pix.z >  abs(pix.y)) {
    colorFromTex(-pix.x,  pix.y,  pix.z, texBack);
    return;
  }

  if (pix.z < -abs(pix.x) && pix.z < -abs(pix.y)) {
    colorFromTex( pix.x,  pix.y, -pix.z, texFront);
    return;
  }

  fragColor = vec4(0.6, 0.3, 0.1, 1);
}
)";

EquirectangularView::EquirectangularView()
  : _impl(std::make_unique<Impl>()) {}

void EquirectangularView::renderFullPipeline(ViewSettings settings) {
  populateViewSettings(settings);

  Eigen::Vector3f pos = Eigen::Vector3f::Zero();
  Eigen::Quaternionf rot = Eigen::Quaternionf::Identity();

  if (settings.viewpoint) {
    pos = settings.viewpoint->getPosition();
    rot = settings.viewpoint->getOrientation();
  }

  _impl->cubemap->renderFullPipeline(settings.renderers, pos, rot);
}

void EquirectangularView::setCubeMapResolution(int res) {
  _impl->cubemap->setCubeMapResolution(res);
}

void EquirectangularView::setLinearInterpolation(bool on) {
  _impl->cubemap->setLinearInterpolation(on);
}

END_NAMESPACE_GMGRAPHICS;
