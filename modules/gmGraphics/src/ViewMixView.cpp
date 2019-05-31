
#include <gmGraphics/ViewMixView.hh>

#include <gmGraphics/GLUtils.hh>
#include <gmCore/RunOnce.hh>

#include <gmGraphics/OffscreenRenderTargets.hh>
#include <gmGraphics/RasterProcessor.hh>

#include <Eigen/Eigen>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE_SUB(ViewMixView, View);
GM_OFI_POINTER(ViewMixView, view, View, ViewMixView::addView);

struct ViewMixView::Impl {

  static const std::string fragment_code;

  OffscreenRenderTargets render_target;
  RasterProcessor raster_processor;

  bool is_setup = false;
  bool is_functional = false;

  void renderFullPipeline(ViewSettings settings);

  std::vector<std::shared_ptr<View>> views;
};

const std::string
ViewMixView::
Impl::fragment_code =
  { R"lang=glsl(
#version 330 core

uniform sampler2D tex[];
uniform int tex_count;

in vec2 position;

out vec4 fragColor;

void main() {
  vec3 rgb = texture(tex[0], position * 0.5 + 0.5).rgb;
  if (tex_count > 1)
    rgb = rgb - texture(tex[1], position * 0.5 + 0.5).rgb;
  fragColor = vec4(abs(rgb), 1);
}
)lang=glsl"
  };

ViewMixView::ViewMixView()
  : _impl(std::make_unique<Impl>()) {}

ViewMixView::~ViewMixView() {}

void ViewMixView::addView(std::shared_ptr<View> v) {
  _impl->views.push_back(v);
}

void ViewMixView::renderFullPipeline(ViewSettings settings) {
  populateViewSettings(settings);
  _impl->renderFullPipeline(settings);
}

void ViewMixView::Impl::renderFullPipeline(ViewSettings settings) {

  if (views.empty()) {
    GM_RUNONCE(GM_ERR("ViewMixView", "Missing views to mix."));
    return;
  }

  if (!is_setup) {
    is_setup = true;
    raster_processor.setFragmentCode(fragment_code);
    if (render_target.init(views.size()) &&
        raster_processor.init())
      is_functional = true;
  }

  if (!is_functional) {
    GM_RUNONCE(GM_ERR("ViewMixView", "Dysfunctional internal GL workings."));
    return;
  }

  render_target.push();

  GM_VINF("ViewMixView", "Render " << views.size() << " views.");
  size_t idx = 0;
  for (auto view : views) {
    render_target.bind(0, 0, idx++);
    view->renderFullPipeline(settings);
  }

  render_target.pop();

  // Render offscreen buffer to active render target

  GM_VINF("ViewMixView", "Render offscreen buffers to active render target");

  for (size_t idx = 0; idx < views.size(); ++idx) {
    glActiveTexture(GL_TEXTURE0 + idx);
    glBindTexture(GL_TEXTURE_2D, render_target.getTexId(idx));
  }

  GLuint program_id = raster_processor.getProgramId();
  glUseProgram(program_id);
  static const GLint tex[] = { 0, 1, 2, 3, 4, 5, 6, 7 };
  glUniform1iv(glGetUniformLocation(program_id, "tex"), 8, tex);
  glUniform1i(glGetUniformLocation(program_id, "tex_count"), views.size());

  raster_processor.run();

  for (size_t idx = 0; idx < views.size(); ++idx) {
    glActiveTexture(GL_TEXTURE0 + idx);
    glBindTexture(GL_TEXTURE_2D, 0);
  }
}

END_NAMESPACE_GMGRAPHICS;
