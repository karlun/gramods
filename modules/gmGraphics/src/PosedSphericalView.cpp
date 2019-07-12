
#include <gmGraphics/PosedSphericalView.hh>

#include <gmGraphics/CubeMapRasterProcessor.hh>
#include <gmGraphics/GLUtils.hh>

#include <gmCore/RunOnce.hh>

#include <GL/glew.h>
#include <GL/gl.h>

#include <vector>
#include <Eigen/Eigen>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE_SUB(PosedSphericalView, View);
GM_OFI_PARAM(PosedSphericalView, cubeMapResolution, int, PosedSphericalView::setCubeMapResolution);
GM_OFI_PARAM(PosedSphericalView, linearInterpolation, bool, PosedSphericalView::setLinearInterpolation);
GM_OFI_PARAM(PosedSphericalView, makeSquare, bool, PosedSphericalView::setMakeSquare);
GM_OFI_POINTER(PosedSphericalView, coordinatesMapper, gmGraphics::CoordinatesMapper, PosedSphericalView::setCoordinatesMapper);

struct PosedSphericalView::Impl {

  static const std::string fragment_template_code;
  static const std::string mapper_pattern;

  std::string createFragmentCode();
  void renderFullPipeline(ViewSettings settings);

  bool make_square = false;

  std::unique_ptr<CubeMapRasterProcessor> cubemap;
  std::shared_ptr<CoordinatesMapper> mapper;

  Impl() : cubemap(std::make_unique<CubeMapRasterProcessor>()) {}

  bool is_setup = false;
};

const std::string PosedSphericalView::Impl::mapper_pattern = "MAPPER;";

const std::string PosedSphericalView::Impl::fragment_template_code = R"lang=glsl(
#version 330 core

uniform sampler2D texLeft;
uniform sampler2D texRight;
uniform sampler2D texBottom;
uniform sampler2D texTop;
uniform sampler2D texBack;
uniform sampler2D texFront;

in vec2 pos;
out vec4 fragColor;

MAPPER;

void colorFromTex(float x, float y, float z, sampler2D tex) {
  fragColor = texture(tex, 0.5 * vec2(x/z, y/z) + 0.5);
}

void main() {

  vec3 pix;
  bool good = mapTo3D(pos, pix);

  if (!good) {
    fragColor = vec4(0, 0, 0, 0);
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
  _impl->renderFullPipeline(settings);
}

void PosedSphericalView::Impl::renderFullPipeline(ViewSettings settings) {

  if (!mapper) {
    GM_RUNONCE(GM_ERR("PosedSphericalView", "No coordinate mapper specified."));
    return;
  }

  if (!is_setup) {
    cubemap->setFragmentCode(createFragmentCode());
    is_setup = true;
  }

  GLint program_id = cubemap->getProgram();
  if (program_id) {
    glUseProgram(program_id);
    mapper->setMapperUniforms(program_id);
  }

  Eigen::Vector3f eye_pos = Eigen::Vector3f::Zero();
  Eigen::Quaternionf head_rot = Eigen::Quaternionf::Identity();

  if (settings.viewpoint) {
    eye_pos = settings.viewpoint->getPosition();
    head_rot = settings.viewpoint->getOrientation();
  }

  cubemap->renderFullPipeline(settings.renderers, eye_pos, head_rot, Eye::MONO, make_square);
}

std::string PosedSphericalView::Impl::createFragmentCode() {
  assert(mapper);
  assert(fragment_template_code.find(mapper_pattern) != std::string::npos);

  std::string mapper_code = mapper->getMapperCode();
  std::string fragment_code = fragment_template_code;

  fragment_code.replace(fragment_code.find(mapper_pattern),
                        mapper_pattern.length(),
                        mapper_code);

  return fragment_code;
}

void PosedSphericalView::setCoordinatesMapper(std::shared_ptr<CoordinatesMapper> mapper) {
  _impl->mapper = mapper;
}

void PosedSphericalView::setMakeSquare(bool on) {
  _impl->make_square = on;
}

void PosedSphericalView::setCubeMapResolution(int res) {
  _impl->cubemap->setCubeMapResolution(res);
}

void PosedSphericalView::setLinearInterpolation(bool on) {
  _impl->cubemap->setLinearInterpolation(on);
}

END_NAMESPACE_GMGRAPHICS;
