
#include <gmGraphics/ViewMixView.hh>

#include <gmGraphics/GLUtils.hh>
#include <gmCore/RunOnce.hh>

#include <gmGraphics/OffscreenRenderTargets.hh>
#include <gmGraphics/RasterProcessor.hh>

#include <Eigen/Eigen>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE_SUB(ViewMixView, View);
GM_OFI_PARAM2(ViewMixView, mixType, std::string, setMixType);
GM_OFI_POINTER2(ViewMixView, view, View, addView);

struct ViewMixView::Impl {

  static const std::string fragment_code[];
  size_t type = 0;

  OffscreenRenderTargets render_target;
  RasterProcessor raster_processor;

  bool is_setup = false;
  bool is_functional = false;

  void renderFullPipeline(ViewSettings settings);

  std::vector<std::shared_ptr<View>> views;
};

const std::string
ViewMixView::
Impl::fragment_code[] =
  { R"lang=glsl(
#version 330 core

uniform sampler2D tex[];
uniform int tex_count;

in vec2 position;

out vec4 fragColor;

void main() {

  float mix = 1.0 / tex_count;

  vec3 rgb = mix * texture(tex[0], position * 0.5 + 0.5).rgb;
  if (tex_count > 1)
    rgb += mix * texture(tex[1], position * 0.5 + 0.5).rgb;
  if (tex_count > 2)
    rgb += mix * texture(tex[2], position * 0.5 + 0.5).rgb;
  if (tex_count > 3)
    rgb += mix * texture(tex[3], position * 0.5 + 0.5).rgb;
  if (tex_count > 4)
    rgb += mix * texture(tex[4], position * 0.5 + 0.5).rgb;
  if (tex_count > 5)
    rgb += mix * texture(tex[5], position * 0.5 + 0.5).rgb;
  if (tex_count > 6)
    rgb += mix * texture(tex[6], position * 0.5 + 0.5).rgb;
  if (tex_count > 7)
    rgb += mix * texture(tex[7], position * 0.5 + 0.5).rgb;

  fragColor = vec4(rgb, 1);
}
)lang=glsl",
    R"lang=glsl(
#version 330 core

uniform sampler2D tex[];
uniform int tex_count;

in vec2 position;

out vec4 fragColor;

void main() {
  vec3 rgb = texture(tex[0], position * 0.5 + 0.5).rgb - texture(tex[1], position * 0.5 + 0.5).rgb;
  fragColor = vec4(abs(rgb), 1);
}
)lang=glsl",
    R"lang=glsl(
#version 330 core

uniform sampler2D tex[];
uniform int tex_count;

in vec2 position;

out vec4 fragColor;

void main() {
  vec3 rgb0 = texture(tex[0], position * 0.5 + 0.5).rgb;
  vec3 rgb1 = texture(tex[1], position * 0.5 + 0.5).rgb;
  float dist = length(rgb1 - rgb0);
  fragColor = vec4(dist, dist, dist, 1);
}
)lang=glsl"
  };

ViewMixView::ViewMixView()
  : _impl(std::make_unique<Impl>()) {}

ViewMixView::~ViewMixView() {}

void ViewMixView::addView(std::shared_ptr<View> v) {
  if (!v) throw gmCore::InvalidArgument("null not allowed");
  _impl->views.push_back(v);
}

  /**
     Set type of mixing. Valid values are

     - average, showing the per pixel average value of all the views,

     - difference, showing the per pixel difference between the first
       and the second added view.

     - distance, showing the per pixel color distance between the
       first and the second added view.
  */
void ViewMixView::setMixType(std::string s) {

  if (_impl->is_setup)
    GM_WRN("ViewMixView", "Settings mix type after initialization will have no effect.");

  if (s == "average") {
    _impl->type = 0;
  }
  else if (s == "difference") {
    _impl->type = 1;
  }
  else if (s == "distance") {
    _impl->type = 2;
  }
  else {
    GM_WRN("ViewMixView", "Unrecognized mix type '" << s << "' - applying average instead.");
    _impl->type = 0;
  }
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

  if (type > 0 && views.size() < 2) {
    GM_RUNONCE(GM_ERR("ViewMixView", "Too few views to mix for the current operator."));
    return;
  }

  if (type > 0 && views.size() > 2) {
    GM_RUNONCE(GM_WRN("ViewMixView", "Too many views to mix for the current operator - residual view will be ignored."));
  }

  if (!is_setup) {
    is_setup = true;
    raster_processor.setFragmentCode(fragment_code[type]);
    render_target.setPixelFormat(settings.pixel_format);
    if (render_target.init(views.size()) &&
        raster_processor.init())
      is_functional = true;
  }

  if (!is_functional) {
    GM_RUNONCE(GM_ERR("ViewMixView", "Dysfunctional internal GL workings."));
    return;
  }

  render_target.push();

  GM_DBG2("ViewMixView", "Render " << views.size() << " views.");
  size_t idx = 0;
  for (auto view : views) {
    render_target.bind(0, 0, idx++);
    view->renderFullPipeline(settings);
  }

  render_target.pop();

  // Render offscreen buffer to active render target

  GM_DBG2("ViewMixView", "Render offscreen buffers to active render target");

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

void ViewMixView::traverse(Visitor *visitor) {
  for (auto &v : _impl->views) v->accept(visitor);
}

END_NAMESPACE_GMGRAPHICS;
