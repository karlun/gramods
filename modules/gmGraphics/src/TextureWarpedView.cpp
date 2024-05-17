
#include <gmGraphics/TextureWarpedView.hh>

#include <gmGraphics/GLUtils.hh>
#include <gmCore/RunOnce.hh>

#include <gmGraphics/OffscreenRenderTargets.hh>
#include <gmGraphics/RasterProcessor.hh>

#include <Eigen/Eigen>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE_SUB(TextureWarpedView, View);
GM_OFI_POINTER2(TextureWarpedView, texture, gmGraphics::TextureInterface, setTexture);
GM_OFI_POINTER2(TextureWarpedView, view, gmGraphics::View, addView);
GM_OFI_PARAM2(TextureWarpedView, edgeToEdge, bool, setEdgeToEdge);
GM_OFI_PARAM2(TextureWarpedView, warpScale, Eigen::Vector2f, setWarpScale);
GM_OFI_PARAM2(TextureWarpedView, warpOffset, Eigen::Vector2f, setWarpOffset);

struct TextureWarpedView::Impl {

  static const std::string fragment_code;

  OffscreenRenderTargets render_target;
  RasterProcessor raster_processor;

  bool is_setup = false;
  bool is_functional = false;

  void renderFullPipeline(ViewSettings settings);

  std::vector<std::shared_ptr<View>> views;
  std::shared_ptr<TextureInterface> texture;
  bool edgeToEdge = false;
  Eigen::Vector2f scale = Eigen::Vector2f::Ones();
  Eigen::Vector2f offset = Eigen::Vector2f::Zero();
};

const std::string
TextureWarpedView::
Impl::fragment_code = R"lang=glsl(
#version 330 core

uniform sampler2D view_id;
uniform sampler2D warp_id;
uniform bool edge_to_edge;
uniform vec2 scale;
uniform vec2 offset;

in vec2 position;

out vec4 fragColor;

void main() {
  vec2 pos = position * 0.5 + 0.5;

  if (edge_to_edge) {
    ivec2 warp_size = textureSize(warp_id, 0);
    vec2 pix_w = 1 / warp_size;
    pos = pos * (1 - pix_w) + 0.5 * pix_w;
  }

  vec2 warp_coord = texture(warp_id, pos).xy;
  warp_coord = warp_coord * scale + offset;

  if (edge_to_edge) {
    ivec2 view_size = textureSize(view_id, 0);
    vec2 pix_w = 1 / view_size;
    warp_coord = warp_coord * (1 - pix_w) + 0.5 * pix_w;
  }

  fragColor = texture(view_id, warp_coord);
}
)lang=glsl";

TextureWarpedView::TextureWarpedView()
  : _impl(std::make_unique<Impl>()) {}

TextureWarpedView::~TextureWarpedView() {}

void TextureWarpedView::addView(std::shared_ptr<View> view) {
  if (!view) throw gmCore::InvalidArgument("specified view is null");
  _impl->views.push_back(view);
}

void TextureWarpedView::renderFullPipeline(ViewSettings settings) {
  populateViewSettings(settings);
  _impl->renderFullPipeline(settings);
}

void TextureWarpedView::Impl::renderFullPipeline(ViewSettings settings) {

  bool preconditions = true;

  if (views.empty()) {
    GM_RUNONCE(GM_ERR("TextureWarpedView", "Missing view to warp."));
    preconditions = false;
  }

  if (!texture) {
    GM_RUNONCE(GM_ERR("TextureWarpedView", "Missing texture with warp coordinates."));
    preconditions = false;
  }

  if (!preconditions) return;

  if (!is_setup) {
    is_setup = true;
    raster_processor.setFragmentCode(fragment_code);
    render_target.setPixelFormat(settings.pixel_format);
    if (render_target.init() &&
        raster_processor.init())
      is_functional = true;
  }

  if (!is_functional) {
    GM_RUNONCE(GM_ERR("TextureWarpedView", "Dysfunctional internal GL workings."));
    return;
  }

  GM_DBG2("TextureWarpedView", "Rendering view");

  render_target.push();
  render_target.bind();
  for (auto view : views)
    view->renderFullPipeline(settings);
  render_target.pop();

  // Render offscreen buffer to active render target

  GM_DBG2("TextureWarpedView", "Render offscreen buffer to active render target");

  glDisable(GL_DEPTH_TEST);

  GLuint warp_id = texture->updateTexture(settings.frame_number, Eye::MONO);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, render_target.getTexId());
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, warp_id);

  GLuint program_id = raster_processor.getProgramId();
  glUseProgram(program_id);
  glUniform1i(glGetUniformLocation(program_id, "view_id"), 0);
  glUniform1i(glGetUniformLocation(program_id, "warp_id"), 1);
  glUniform1i(glGetUniformLocation(program_id, "edge_to_edge"), edgeToEdge);
  glUniform2fv(glGetUniformLocation(program_id, "scale"), 1, scale.data());
  glUniform2fv(glGetUniformLocation(program_id, "offset"), 1, offset.data());

  raster_processor.run();

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, 0);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, 0);
}

void TextureWarpedView::setTexture(std::shared_ptr<TextureInterface> tex) {
  _impl->texture = tex;
}

void TextureWarpedView::setEdgeToEdge(bool on) {
  _impl->edgeToEdge = on;
}

void TextureWarpedView::setWarpScale(Eigen::Vector2f scale) {
  _impl->scale = scale;
}

void TextureWarpedView::setWarpOffset(Eigen::Vector2f offset) {
  _impl->offset = offset;
}

void TextureWarpedView::traverse(Visitor *visitor) {
  if (auto obj = std::dynamic_pointer_cast<gmCore::Object>(_impl->texture))
    obj->accept(visitor);
  for (auto &v : _impl->views) v->accept(visitor);
}

END_NAMESPACE_GMGRAPHICS;
