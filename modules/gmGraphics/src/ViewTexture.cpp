
#include <gmGraphics/ViewTexture.hh>

#ifdef gramods_ENABLE_FreeImage

#include <gmGraphics/FreeImage.hh>
#include <gmCore/RunOnce.hh>

#include <gmGraphics/OffscreenRenderTargets.hh>
#include <gmGraphics/RasterProcessor.hh>

#include <GL/glew.h>
#include <GL/gl.h>

#include <chrono>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE_SUB(ViewTexture, View);
GM_OFI_PARAM(ViewTexture, resolution, gmCore::size2, ViewTexture::setResolution);
GM_OFI_PARAM(ViewTexture, useAlpha, bool, ViewTexture::setUseAlpha);
GM_OFI_PARAM(ViewTexture, useFloat, bool, ViewTexture::setUseFloat);
GM_OFI_POINTER(ViewTexture, view, gmGraphics::View, ViewTexture::addView);

struct ViewTexture::Impl {

  bool use_alpha = false;
  bool use_float = false;

  static const std::string fragment_code;
  gmCore::size2 resolution = { 1024, 1024 };

  OffscreenRenderTargets render_target;
  RasterProcessor raster_processor;
  GLenum pixel_format;

  std::vector<std::shared_ptr<View>> views;
  int frame = 0;

  bool is_setup = false;
  bool is_functional = false;

  bool setup();
  void update(ViewSettings settings);
  void renderFullPipeline(ViewSettings settings);
};

ViewTexture::ViewTexture()
  : _impl(std::make_unique<Impl>()) {}

const std::string ViewTexture::Impl::fragment_code =
  R"lang=glsl(
#version 330 core

uniform sampler2D tex;

in vec2 position;

out vec4 fragColor;

void main() {

  vec3 rgb = texture(tex, position * 0.5 + 0.5).rgb;

  fragColor = vec4(rgb, 1);
}
)lang=glsl";

bool ViewTexture::Impl::setup() {
  if (views.empty()) {
    GM_RUNONCE(GM_ERR("ViewTexture", "No view to save."));
    return false;
  }

  if (resolution[0] <= 0 || resolution[1] <= 0) {
    GM_RUNONCE(GM_ERR("ViewTexture", "Cannot create view of resolution " << resolution[0] << "x" << resolution[1] << "."));
    return false;
  }

  if (!is_setup) {
    is_setup = true;
    pixel_format = use_float ?
      use_alpha ? GL_RGBA32F : GL_RGB32F :
      use_alpha ? GL_RGBA8 : GL_RGB8;
    render_target.setPixelFormat(pixel_format);
    raster_processor.setFragmentCode(fragment_code);
    if (render_target.init(1) &&
        raster_processor.init())
      is_functional = true;
  }

  if (!is_functional) {
    GM_RUNONCE(GM_ERR("ViewTexture", "Dysfunctional internal GL workings."));
    return false;
  }

  return true;
}

void ViewTexture::renderFullPipeline(ViewSettings settings) {
  populateViewSettings(settings);
  _impl->renderFullPipeline(settings);
}

void ViewTexture::Impl::renderFullPipeline(ViewSettings settings) {

  if (!setup()) return;

  update(settings);

  // Render offscreen buffer to active render target

  GM_VINF("ViewTexture", "Render offscreen buffers to active render target");

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, render_target.getTexId());

  GLuint program_id = raster_processor.getProgramId();
  glUseProgram(program_id);
  static const GLint tex = 0;
  glUniform1iv(glGetUniformLocation(program_id, "tex"), 1, &tex);

  raster_processor.run();

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, 0);
}

void ViewTexture::update() {
  if (!_impl->setup()) return;

  ViewSettings settings(viewpoint);
  populateViewSettings(settings);
  _impl->update(settings);
}

void ViewTexture::Impl::update(ViewSettings settings) {
  render_target.push();
  render_target.bind(resolution[0], resolution[1]);

  settings.pixel_format = pixel_format;
  for (auto view : views)
    view->renderFullPipeline(settings);

  render_target.pop();
}

void ViewTexture::setResolution(gmCore::size2 res) {
  _impl->resolution = res;
}

void ViewTexture::setUseFloat(bool on) {
  _impl->use_float = on;
}

bool ViewTexture::getUseFloat() {
  return _impl->use_float;
}

void ViewTexture::setUseAlpha(bool on) {
  _impl->use_alpha = on;
}

bool ViewTexture::getUseAlpha() {
  return _impl->use_alpha;
}

void ViewTexture::addView(std::shared_ptr<View> view) {
  _impl->views.push_back(view);
}

void ViewTexture::clearRenderers(bool recursive) {
  if (recursive)
    for (auto view : _impl->views)
      view->clearRenderers(recursive);
  RendererDispatcher::clearRenderers(recursive);
}

GLuint ViewTexture::getGLTextureID() {
  return _impl->render_target.getTexId();
}

END_NAMESPACE_GMGRAPHICS;

#endif
