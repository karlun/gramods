
#include <gmGraphics/MixingView.hh>

#include <gmGraphics/GLUtils.hh>
#include <gmCore/RunOnce.hh>

#include <gmGraphics/MixingShaders.hh>
#include <gmGraphics/OffscreenRenderTargets.hh>
#include <gmGraphics/RasterProcessor.hh>

#include <Eigen/Eigen>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE_SUB(MixingView, View);
GM_OFI_PARAM2(MixingView, mixType, std::string, setMixType);
GM_OFI_POINTER2(MixingView, view, View, addView);

struct MixingView::Impl {

  std::string mix_type = "average";

  OffscreenRenderTargets render_target;
  RasterProcessor raster_processor;

  bool is_setup = false;
  bool is_functional = false;

  void renderFullPipeline(ViewSettings settings);

  std::vector<std::shared_ptr<View>> views;
};

MixingView::MixingView() : _impl(std::make_unique<Impl>()) {}

MixingView::~MixingView() {}

void MixingView::addView(std::shared_ptr<View> v) {
  if (!v) throw gmCore::InvalidArgument("null not allowed");
  _impl->views.push_back(v);
}

void MixingView::setMixType(std::string str) {

  if (_impl->is_setup)
    GM_WRN("MixingView",
           "Settings mix type after initialization will have no effect.");

  if (MixingShaders::checkMixTypeValid(str, "MixingView"))
    _impl->mix_type = str;
}

void MixingView::renderFullPipeline(ViewSettings settings) {
  populateViewSettings(settings);
  _impl->renderFullPipeline(settings);
}

void MixingView::Impl::renderFullPipeline(ViewSettings settings) {

  if (views.empty()) {
    GM_RUNONCE(GM_ERR("MixingView", "Missing views to mix."));
    return;
  }

  if (!MixingShaders::checkCount(mix_type, views.size(), "MixingView", "views"))
    return;

  if (!is_setup) {
    is_setup = true;
    raster_processor.setFragmentCode(MixingShaders::getFragmentCode(
        mix_type, {"rgb", "rgb", "rgb", "rgb", "rgb", "rgb", "rgb", "rgb"}));
    render_target.setPixelFormat(settings.pixel_format);
    if (render_target.init(views.size()) && raster_processor.init())
      is_functional = true;
  }

  if (!is_functional) {
    GM_RUNONCE(GM_ERR("MixingView", "Dysfunctional internal GL workings."));
    return;
  }

  render_target.push();

  GM_DBG2("MixingView", "Render " << views.size() << " views.");
  size_t idx = 0;
  for (auto view : views) {
    render_target.bind(0, 0, idx++);
    view->renderFullPipeline(settings);
  }

  render_target.pop();

  // Render offscreen buffer to active render target

  GM_DBG2("MixingView", "Render offscreen buffers to active render target");

  for (size_t idx = 0; idx < views.size(); ++idx) {
    glActiveTexture(GL_TEXTURE0 + idx);
    glBindTexture(GL_TEXTURE_2D, render_target.getTexId(idx));
  }

  GLuint program_id = raster_processor.getProgramId();
  glUseProgram(program_id);
  static const GLint tex[] = {0, 1, 2, 3, 4, 5, 6, 7};
  glUniform1iv(glGetUniformLocation(program_id, "tex"), 8, tex);
  glUniform1i(glGetUniformLocation(program_id, "tex_count"), views.size());

  raster_processor.run();

  for (size_t idx = 0; idx < views.size(); ++idx) {
    glActiveTexture(GL_TEXTURE0 + idx);
    glBindTexture(GL_TEXTURE_2D, 0);
  }
}

void MixingView::traverse(Visitor *visitor) {
  for (auto &v : _impl->views) v->accept(visitor);
}

END_NAMESPACE_GMGRAPHICS;
