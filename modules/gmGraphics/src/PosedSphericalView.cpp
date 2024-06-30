
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
GM_OFI_PARAM2(PosedSphericalView, cubeMapResolution, int, setCubeMapResolution);
GM_OFI_PARAM2(PosedSphericalView, linearInterpolation, bool, setLinearInterpolation);
GM_OFI_PARAM2(PosedSphericalView, makeSquare, bool, setMakeSquare);
GM_OFI_POINTER2(PosedSphericalView, coordinatesMapper, gmGraphics::CoordinatesMapper, setCoordinatesMapper);

struct PosedSphericalView::Impl {

  static const std::string fragment_template_code;
  static const std::string mapper_pattern;

  std::string createFragmentCode();
  void renderFullPipeline(ViewSettings settings);
  void renderFullPipeline(ViewSettings settings, Viewpoint *vp);

  bool make_square = false;

  std::unique_ptr<CubeMapRasterProcessor> cubemap;
  std::shared_ptr<CoordinatesMapper> mapper;

  Impl() : cubemap(std::make_unique<CubeMapRasterProcessor>()) {}

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

  for (auto viewpoint : settings.viewpoints)
    renderFullPipeline(settings, viewpoint.get());
}

void PosedSphericalView::Impl::renderFullPipeline(ViewSettings settings,
                                                  Viewpoint *viewpoint) {

  Eigen::Vector3f eye_pos = viewpoint->getPosition();
  Eigen::Quaternionf head_rot = viewpoint->getOrientation();

  GLint program_id = cubemap->getProgram();
  if (!program_id) {

    cubemap->setFragmentCode(createFragmentCode());

    ViewSettings empty_settings(settings.frame_number);
    cubemap->renderFullPipeline(
        empty_settings, eye_pos, head_rot, Eye::MONO, make_square);
    program_id = cubemap->getProgram();

    if (!program_id) {
      GM_RUNONCE(GM_ERR("SpatialSphericalView", "Could not initialize cubemap"));
      return;
    }
  }

  glUseProgram(program_id);
  mapper->setCommonUniforms(program_id);
  mapper->setTo3DUniforms(program_id);
  glUseProgram(0);

  cubemap->renderFullPipeline(settings,
                              eye_pos,
                              head_rot,
                              Eye::MONO,
                              make_square);
}

std::string PosedSphericalView::Impl::createFragmentCode() {
  assert(mapper);
  assert(fragment_template_code.find(mapper_pattern) != std::string::npos);

  std::string mapper_code =
      mapper->getCommonCode() + "\n" + mapper->getTo3DCode();
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
  if (on) {
    GM_RUNONCE(
        GM_WRN("PosedSphericalView",
               "MakeSquare set to true; using square view instead is safer!"));
  }

  _impl->make_square = on;
}

void PosedSphericalView::setCubeMapResolution(int res) {
  _impl->cubemap->setCubeMapResolution(res);
}

void PosedSphericalView::setLinearInterpolation(bool on) {
  _impl->cubemap->setLinearInterpolation(on);
}

void PosedSphericalView::traverse(Visitor *visitor) {
  if (_impl->mapper) _impl->mapper->accept(visitor);
}

END_NAMESPACE_GMGRAPHICS;
