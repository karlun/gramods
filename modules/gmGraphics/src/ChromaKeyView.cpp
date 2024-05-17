
#include <gmGraphics/ChromaKeyView.hh>

#include <gmGraphics/GLUtils.hh>
#include <gmCore/RunOnce.hh>
#include <gmCore/Console.hh>

#include <gmGraphics/OffscreenRenderTargets.hh>
#include <gmGraphics/RasterProcessor.hh>

#include <Eigen/Eigen>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE_SUB(ChromaKeyView, View);
GM_OFI_PARAM2(ChromaKeyView, key, gmCore::float3, setKey);
GM_OFI_PARAM2(ChromaKeyView, tolerance, gmCore::float2, setTolerance);
GM_OFI_POINTER2(ChromaKeyView, view, View, addView);

struct ChromaKeyView::Impl {

  static const std::string fragment_code;

  OffscreenRenderTargets render_target;
  RasterProcessor raster_processor;

  bool is_setup = false;
  bool is_functional = false;

  void renderFullPipeline(ViewSettings settings);

  std::vector<std::shared_ptr<View>> views;
  gmCore::float3 key = { 0.f, 1.f, 0.f };
  gmCore::float2 tolerance = { 0.48f, 0.5f };
};

const std::string
ChromaKeyView::
Impl::fragment_code = R"lang=glsl(
#version 330 core

uniform sampler2D tex;

uniform vec3 key_rgb;

uniform float r0;
uniform float r1;

in vec2 position;

out vec4 fragColor;

vec3 rgb2ycbcr (vec3 rgb) {
  return vec3(0.299 * rgb.r + 0.587 * rgb.g + 0.114 * rgb.b,
              0.5 - 0.168736 * rgb.r - 0.331264 * rgb.g + 0.5 * rgb.b,
              0.5 + 0.5 * rgb.r - 0.418688 * rgb.g - 0.081312 * rgb.b);
}

float maskvalue(vec3 frag, float r0, float r1) {

  vec3 key_cc = rgb2ycbcr(key_rgb);
  vec3 frag_cc = rgb2ycbcr(frag);

  vec2 CbCr_diff = key_cc.yz - frag_cc.yz;
  float CbCr_dist = length(CbCr_diff);

  if (CbCr_dist < r0) return 0.0;
  if (CbCr_dist > r1) return 1.0;

  return (CbCr_dist - r0) / (r1 - r0);
}

vec4 chroma_key(vec4 color) {
  float mask = maskvalue(color.rgb, r0, r1);
  return vec4(color.rgb - (1 - mask) * key_rgb, mask * color.a);
}

void main() {
  fragColor = chroma_key(texture(tex, position * 0.5 + 0.5));
}
)lang=glsl";

ChromaKeyView::ChromaKeyView()
  : _impl(std::make_unique<Impl>()) {}

ChromaKeyView::~ChromaKeyView() {}

void ChromaKeyView::addView(std::shared_ptr<View> view) {
  if (!view) throw gmCore::InvalidArgument("null not allowed");
  _impl->views.push_back(view);
}

void ChromaKeyView::renderFullPipeline(ViewSettings settings) {
  populateViewSettings(settings);
  _impl->renderFullPipeline(settings);
}

void ChromaKeyView::Impl::renderFullPipeline(ViewSettings settings) {

  if (views.empty()) {
    GM_RUNONCE(GM_ERR("ChromaKeyView", "Missing view to mask by chroma key."));
    return;
  }

  if (!is_setup) {
    is_setup = true;
    raster_processor.setFragmentCode(fragment_code);
    render_target.setPixelFormat(settings.pixel_format);
    if (render_target.init() &&
        raster_processor.init())
      is_functional = true;
  }

  if (!is_functional) {
    GM_RUNONCE(GM_ERR("ChromaKeyView", "Dysfunctional internal GL workings."));
    return;
  }

  GM_DBG2("ChromaKeyView", "Rendering view");

  render_target.push();
  render_target.bind();
  for (auto view : views)
    view->renderFullPipeline(settings);
  render_target.pop();

  // Render offscreen buffer to active render target

  GM_DBG2("ChromaKeyView", "Render offscreen buffer to active render target");

  glDisable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, render_target.getTexId());

  GLuint program_id = raster_processor.getProgramId();
  glUseProgram(program_id);
  glUniform1i(glGetUniformLocation(program_id, "tex"), 0);
  glUniform3fv(glGetUniformLocation(program_id, "key_rgb"), 1, key.data());
  glUniform1f(glGetUniformLocation(program_id, "r0"), tolerance[0]);
  glUniform1f(glGetUniformLocation(program_id, "r1"), tolerance[1]);

  raster_processor.run();

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, 0);
}

void ChromaKeyView::setKey(gmCore::float3 key) {
  _impl->key = key;
}

void ChromaKeyView::setTolerance(gmCore::float2 tol) {
  _impl->tolerance = tol;
}

void ChromaKeyView::traverse(Visitor *visitor) {
  for (auto &v : _impl->views) v->accept(visitor);
}

END_NAMESPACE_GMGRAPHICS;
