
#include <gmGraphics/SaveView.hh>

#ifdef gramods_ENABLE_FreeImage

#include <gmGraphics/FreeImage.hh>
#include <gmCore/RunOnce.hh>

#include <gmGraphics/OffscreenRenderTargets.hh>
#include <gmGraphics/RasterProcessor.hh>

#include <FreeImage.h>

#include <GL/glew.h>
#include <GL/gl.h>

#include <chrono>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE_SUB(SaveView, View);
GM_OFI_PARAM(SaveView, file, std::string, SaveView::setFile);
GM_OFI_PARAM(SaveView, resolution, gmTypes::size2, SaveView::setResolution);
GM_OFI_PARAM(SaveView, useAlpha, bool, SaveView::setUseAlpha);
GM_OFI_PARAM(SaveView, useFloat, bool, SaveView::setUseFloat);
GM_OFI_POINTER(SaveView, view, gmGraphics::View, SaveView::setView);

struct SaveView::Impl {

  bool alpha_support = true;
  bool float_support = false;
  bool use_alpha = false;
  bool use_float = false;
  std::string file_template = "SaveView.png";
  FREE_IMAGE_FORMAT fi_format = FIF_PNG;
  int fi_options = PNG_Z_BEST_SPEED;

  static const std::string fragment_code;
  gmTypes::size2 resolution = { 0, 0 };

  OffscreenRenderTargets render_target;
  RasterProcessor raster_processor;
  GLenum pixel_format;

  std::shared_ptr<View> view;
  int frame = 0;

  std::shared_ptr<FreeImage> free_image;

  bool is_setup = false;
  bool is_functional = false;

  void renderFullPipeline(ViewSettings settings);
};

SaveView::SaveView()
  : _impl(std::make_unique<Impl>()) {
  _impl->free_image = FreeImage::get();
}

const std::string SaveView::Impl::fragment_code =
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

void SaveView::renderFullPipeline(ViewSettings settings) {
  populateViewSettings(settings);
  _impl->renderFullPipeline(settings);
}

void SaveView::Impl::renderFullPipeline(ViewSettings settings) {

  if (!view) {
    GM_RUNONCE(GM_ERR("SaveView", "No view to save."));
    return;
  }

  if (file_template.size() == 0) {
    GM_RUNONCE(GM_ERR("SaveView", "Cannot not save - empty filename"));
    return;
  }

  if (resolution[0] < 0 || resolution[1] < 0) {
    GM_RUNONCE(GM_ERR("SaveView", "Cannot create view of resolution " << resolution[0] << "x" << resolution[1] << "."));
    return;
  }

  if (use_alpha && !alpha_support) {
    GM_WRN("SaveView", "Alpha requested together with file format that does not support alpha. Will not save alpha channel.");
    use_alpha = false;
  }

  if (use_float && !float_support) {
    GM_WRN("SaveView", "Float format requested together with file format that does not support float. Will not save as float.");
    use_float = false;
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
    GM_RUNONCE(GM_ERR("SaveView", "Dysfunctional internal GL workings."));
    return;
  }

  render_target.push();
  render_target.bind(resolution[0], resolution[1]);

  settings.pixel_format = pixel_format;
  view->renderFullPipeline(settings);

  auto t0 = std::chrono::steady_clock::now();

  GLint size[4];
  glGetIntegerv( GL_VIEWPORT, size );

  int width = size[2] - size[0];
  int height = size[3] - size[1];

  int bits_per_channel = -1;
  int number_of_channels = use_alpha ? 4 : 3;
  GLenum channel_type;
  FREE_IMAGE_TYPE fi_image_type;

  if (use_float) {
    fi_image_type = use_alpha ? FIT_RGBAF : FIT_RGBF;
    channel_type = GL_FLOAT;
    bits_per_channel = 32;
  } else {
    fi_image_type = FIT_BITMAP;
    channel_type = GL_UNSIGNED_BYTE;
    bits_per_channel = 8;
  }

  FIBITMAP *bitmap = FreeImage_AllocateT
    (fi_image_type, width, height,
     bits_per_channel * number_of_channels);
  BYTE * bytes = FreeImage_GetBits(bitmap);

  {
    GLint current;
    glGetIntegerv(GL_DRAW_BUFFER, &current);
    glReadBuffer(current);
  }

  glReadPixels(size[0], size[1], width, height,
               use_alpha ? GL_BGRA : GL_BGR,
               channel_type, bytes);

  auto t1 = std::chrono::steady_clock::now();

  size_t filename_size = snprintf(nullptr, 0, file_template.c_str(), frame) + 1;
  std::vector<char> filename(filename_size + 1);
  snprintf(filename.data(), filename_size, file_template.c_str(), frame);
  ++frame;

  FreeImage_Save(fi_format, bitmap, filename.data(), fi_options);
  FreeImage_Unload(bitmap);

  auto t2 = std::chrono::steady_clock::now();

  typedef std::chrono::duration<double, std::ratio<1>> d_seconds;
  auto dt1 = std::chrono::duration_cast<d_seconds>(t1 - t0);
  auto dt2 = std::chrono::duration_cast<d_seconds>(t2 - t1);

  GM_VINF("SaveView", "Captured and saved image " << &filename[0] << " in " << int(1e3 * dt1.count() + 0.8) << " + " << int(1e3 * dt2.count() + 0.8) << " ms");

  render_target.pop();

  // Render offscreen buffer to active render target

  GM_VINF("SaveView", "Render offscreen buffers to active render target");

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

void SaveView::setFile(std::string file) {

#define ENDSWITH(STRING,SUFFIX) \
  ((STRING).rfind(SUFFIX) == (STRING).size() - std::string(SUFFIX).size())

  if (ENDSWITH(file, ".png")) {

    _impl->alpha_support = true;
    _impl->float_support = false;
    _impl->file_template = file;
    _impl->fi_format = FIF_PNG;
    _impl->fi_options = PNG_Z_BEST_SPEED;

    return;

  } else if (ENDSWITH(file, ".jpg") ||
             ENDSWITH(file, ".jpeg") ) {

    _impl->alpha_support = false;
    _impl->float_support = false;
    _impl->file_template = file;
    _impl->fi_format = FIF_JPEG;
    _impl->fi_options = JPEG_QUALITYBAD;

    return;

  } else if (ENDSWITH(file, ".exr")) {

    _impl->alpha_support = false;
    _impl->float_support = true;
    _impl->file_template = file;
    _impl->fi_format = FIF_EXR;
    _impl->fi_options = EXR_DEFAULT;

    return;

  } else if (ENDSWITH(file, ".tif") ||
             ENDSWITH(file, ".tiff")) {

    _impl->alpha_support = true;
    _impl->float_support = true;
    _impl->file_template = file;
    _impl->fi_format = FIF_TIFF;
    _impl->fi_options = TIFF_DEFAULT;

    return;

  } else if (ENDSWITH(file, ".pfm")) {

    _impl->alpha_support = false;
    _impl->float_support = true;
    _impl->file_template = file;
    _impl->fi_format = FIF_PFM;
    _impl->fi_options = 0;

    return;

  } else if (ENDSWITH(file, ".bmp")) {

    _impl->alpha_support = false;
    _impl->float_support = false;
    _impl->file_template = file;
    _impl->fi_format = FIF_BMP;
    _impl->fi_options = BMP_SAVE_RLE;

    return;
  }

  GM_WRN("SaveView", "Missing suffix for a supported file format ('" << file << "') - adding '.png'");

  // Could not find supported suffix - add suffix and call again
  setFile(static_cast<std::stringstream&>
          (std::stringstream() << file << ".png").str());
}

void SaveView::setResolution(gmTypes::size2 res) {
  _impl->resolution = res;
}

void SaveView::setUseFloat(bool on) {
  _impl->use_float = on;
}

bool SaveView::getUseFloat() {
  return _impl->use_float;
}

void SaveView::setUseAlpha(bool on) {
  _impl->use_alpha = on;
}

bool SaveView::getUseAlpha() {
  return _impl->use_alpha;
}

void SaveView::setView(std::shared_ptr<View> view) {
  _impl->view = view;
}

void SaveView::clearRenderers(bool recursive) {
  if (recursive)
    _impl->view->clearRenderers(recursive);
  RendererDispatcher::clearRenderers(recursive);
}

END_NAMESPACE_GMGRAPHICS;

#endif
