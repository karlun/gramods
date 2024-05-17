
#include <gmGraphics/RemapperView.hh>

#include <gmGraphics/GLUtils.hh>
#include <gmCore/RunOnce.hh>

#include <gmGraphics/OffscreenRenderTargets.hh>
#include <gmGraphics/RasterProcessor.hh>

#include <Eigen/Eigen>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE_SUB(RemapperView, View);
GM_OFI_POINTER2(RemapperView, view, View, addView);
GM_OFI_POINTER2(RemapperView, from, CoordinatesMapper, setFrom);
GM_OFI_POINTER2(RemapperView, to, CoordinatesMapper, setTo);
GM_OFI_PARAM2(RemapperView, viewOffset, float, setViewOffset);
GM_OFI_PARAM2(RemapperView, sphereRadius, float, setSphereRadius);
GM_OFI_PARAM2(RemapperView, orientation, gmCore::angle2, setOrientation);

struct RemapperView::Impl {

  static const std::string fragment_template_code;
  static const std::string mapper_pattern;

  OffscreenRenderTargets render_target;
  RasterProcessor raster_processor;

  bool is_setup = false;
  bool is_functional = false;

  std::string createFragmentCode();
  void renderFullPipeline(ViewSettings settings);

  std::vector<std::shared_ptr<View>> views;
  std::shared_ptr<CoordinatesMapper> from;
  std::shared_ptr<CoordinatesMapper> to;

  float view_offset = 0.f;
  float sphere_radius = 1.f;
  Eigen::Matrix3f M_rot = Eigen::Matrix3f::Identity();
};

const std::string RemapperView::Impl::fragment_template_code =
    R"lang=glsl(
#version 330 core

MAPPER;

uniform mat3 M_rot;

uniform sampler2D tex;

in vec2 position;

out vec4 fragColor;

void main() {

  vec3 pix_dir;
  bool good = mapTo3D(position, pix_dir);

  if (!good) {
    fragColor = vec4(0, 0, 0, 0);
    return;
  }

  vec2 v_uv;
  good = mapTo2D(M_rot * pix_dir, v_uv);

  fragColor = texture(tex, 0.5 + 0.5 * v_uv);
}
)lang=glsl";

const std::string RemapperView::Impl::mapper_pattern = "MAPPER;";

RemapperView::RemapperView() : _impl(std::make_unique<Impl>()) {}

RemapperView::~RemapperView() {}

void RemapperView::addView(std::shared_ptr<View> view) {
  if (!view) throw gmCore::InvalidArgument("null not allowed");
  _impl->views.push_back(view);
}

void RemapperView::setFrom(std::shared_ptr<CoordinatesMapper> from) {
  _impl->from = from;
}

void RemapperView::setTo(std::shared_ptr<CoordinatesMapper> to) {
  _impl->to = to;
}

void RemapperView::setViewOffset(float q) {
  _impl->view_offset = q;
  setOrientation({std::atan(_impl->view_offset / _impl->sphere_radius)});
}

void RemapperView::setSphereRadius(float r) {
  if (r < std::numeric_limits<float>::epsilon())
    throw gmCore::InvalidArgument(
        GM_STR("sphereRadius cannot be non-positive, " << r << " given"));
  _impl->sphere_radius = r;
  setOrientation({std::atan(_impl->view_offset / _impl->sphere_radius), 0.f});
}

void RemapperView::setOrientation(gmCore::angle2 a) {
  auto &M_rot = _impl->M_rot;

  M_rot = (Eigen::AngleAxis<float>(a[1], Eigen::Vector3f::UnitX()) *
           Eigen::AngleAxis<float>(a[0], Eigen::Vector3f::UnitY()))
              .matrix();
}

void RemapperView::renderFullPipeline(ViewSettings settings) {
  populateViewSettings(settings);
  _impl->renderFullPipeline(settings);
}

void RemapperView::Impl::renderFullPipeline(ViewSettings settings) {

  bool missing_data = false;
  if (views.empty()) {
    GM_RUNONCE(GM_ERR("RemapperView", "Missing view to remap."));
    missing_data = true;
  }

  if (!from) {
    GM_RUNONCE(GM_ERR("RemapperView", "Missing mapper for source material."));
    missing_data = true;
  }

  if (!to) {
    GM_RUNONCE(GM_ERR("RemapperView", "Missing mapper for result."));
    missing_data = true;
  }

  if (missing_data) return;

  if (!is_setup) {
    is_setup = true;
    raster_processor.setFragmentCode(createFragmentCode());
    render_target.setPixelFormat(settings.pixel_format);
    if (render_target.init() && raster_processor.init()) is_functional = true;
  }

  if (!is_functional) {
    GM_RUNONCE(GM_ERR("RemapperView", "Dysfunctional internal GL workings."));
    return;
  }

  GM_DBG2("RemapperView", "Rendering view");

  render_target.push();
  render_target.bind();
  for (auto view : views) view->renderFullPipeline(settings);
  render_target.pop();

  // Render offscreen buffer to active render target

  GM_DBG2("RemapperView", "Render offscreen buffer to active render target");

  glDisable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, render_target.getTexId());

  GLuint program_id = raster_processor.getProgramId();
  glUseProgram(program_id);
  glUniform1i(glGetUniformLocation(program_id, "tex"), 0);
  glUniformMatrix3fv(
      glGetUniformLocation(program_id, "M_rot"), 1, false, M_rot.data());
  from->setCommonUniforms(program_id);
  from->setTo2DUniforms(program_id);
  to->setCommonUniforms(program_id);
  to->setTo3DUniforms(program_id);

  raster_processor.run();

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, 0);
}

std::string RemapperView::Impl::createFragmentCode() {
  std::string mapper_code;
  mapper_code += from->getCommonCode() + "\n";
  mapper_code += from->getTo2DCode() + "\n";
  mapper_code += to->getCommonCode() + "\n";
  mapper_code += to->getTo3DCode() + "\n";
  std::string fragment_code = fragment_template_code;

  fragment_code.replace(
      fragment_code.find(mapper_pattern), mapper_pattern.length(), mapper_code);

  return fragment_code;
}

void RemapperView::traverse(Visitor *visitor) {
  for (auto &v : _impl->views) v->accept(visitor);
  if (_impl->from) _impl->from->accept(visitor);
  if (_impl->to) _impl->to->accept(visitor);
}

END_NAMESPACE_GMGRAPHICS;
