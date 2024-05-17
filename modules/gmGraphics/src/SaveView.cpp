
#include <gmGraphics/SaveView.hh>

#ifdef gramods_ENABLE_FreeImage

#include <gmCore/FreeImage.hh>
#include <gmCore/RunOnce.hh>
#include <gmCore/Stringify.hh>
#include <gmCore/ExitException.hh>
#include <gmCore/FileResolver.hh>

#include <gmGraphics/OffscreenRenderTargets.hh>
#include <gmGraphics/RasterProcessor.hh>

#include <FreeImage.h>

#include <GL/glew.h>
#include <GL/gl.h>

#include <chrono>
#include <optional>
#include <thread>
#include <condition_variable>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE_SUB(SaveView, View);
GM_OFI_PARAM2(SaveView, file, std::filesystem::path, setFile);
GM_OFI_PARAM2(SaveView, resolution, gmCore::size2, setResolution);
GM_OFI_PARAM2(SaveView, useAlpha, bool, setUseAlpha);
GM_OFI_PARAM2(SaveView, useFloat, bool, setUseFloat);
GM_OFI_PARAM2(SaveView, exit, bool, setExit);
GM_OFI_POINTER2(SaveView, view, gmGraphics::View, addView);

struct SaveView::Impl {

  Impl();
  ~Impl();

  bool alpha_support = true;
  bool float_support = false;
  bool use_alpha = false;
  bool use_float = false;
  bool do_exit = false;
  std::filesystem::path file_template = "SaveView.png";
  FREE_IMAGE_FORMAT fi_format = FIF_PNG;
  int fi_options = PNG_Z_BEST_SPEED;

  static const std::string fragment_code;
  std::optional<gmCore::size2> resolution;

  OffscreenRenderTargets render_target;
  RasterProcessor raster_processor;
  GLenum pixel_format;

  std::vector<std::shared_ptr<View>> views;
  int frame = 0;

  std::shared_ptr<gmCore::FreeImage> free_image;

  bool is_setup = false;
  bool is_functional = false;

  void renderFullPipeline(ViewSettings settings);

  struct FileBuffer {

    FileBuffer(std::string filename,
               FREE_IMAGE_TYPE image_type,
               int width,
               int height,
               int bits_per_pixel)
      : filename(filename) {
      bitmap = FreeImage_AllocateT(image_type, width, height, bits_per_pixel);
    }

    ~FileBuffer() { FreeImage_Unload(bitmap); }

    BYTE *getBytes() { return FreeImage_GetBits(bitmap); }

    FIBITMAP *bitmap;
    std::string filename;
  };

  void saveImage(std::unique_ptr<FileBuffer> image);
  void save_process();

  bool save_process_alive = true;
  std::thread save_thread;
  std::condition_variable save_condition;
  std::mutex save_lock;
  std::unique_ptr<FileBuffer> save_image;
};

SaveView::SaveView()
  : _impl(std::make_unique<Impl>()) {
  _impl->free_image = gmCore::FreeImage::get();
}

SaveView::Impl::Impl() {
  save_thread = std::thread([this]{ this->save_process(); });
}

SaveView::Impl::~Impl() {

  {
    std::unique_lock<std::mutex> guard(save_lock);
    save_process_alive = false;
    save_condition.notify_all();
  }

  if (!save_thread.joinable()) return;

  try {
    save_thread.join();
  } catch (const std::invalid_argument &e) {
    GM_DBG2("SaveView", "Could not join save thread: " << e.what() << ".");
  } catch (const std::system_error &e) {
    GM_WRN("SaveView",
           "Caught system_error while joining save thread. Code "
           << e.code() << " meaning " << e.what() << ".");
  }
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

  if (views.empty()) {
    GM_RUNONCE(GM_ERR("SaveView", "No view to save."));
    return;
  }

  if (file_template.empty()) {
    GM_RUNONCE(GM_ERR("SaveView", "Cannot not save - empty filename"));
    return;
  }

  if (!resolution) {
    std::array<GLint, 4> viewport;
    glGetIntegerv(GL_VIEWPORT, viewport.data());
    resolution = {size_t(viewport[2]), size_t(viewport[3])};
  }

  if ((*resolution)[0] == 0 || (*resolution)[1] == 0) {
    GM_RUNONCE(GM_ERR("SaveView", "Cannot create zero size view (0x0)"));
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
      GL_RGBA32F : GL_RGBA8;
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
  render_target.bind(*resolution);

  settings.pixel_format = pixel_format;
  for (auto view : views)
    view->renderFullPipeline(settings);

  auto t0 = std::chrono::steady_clock::now();

  GLint size[4];
  glGetIntegerv( GL_VIEWPORT, size );

  int width = size[2];
  int height = size[3];

  size_t filename_size =
      snprintf(nullptr, 0, file_template.u8string().c_str(), frame);
  std::vector<char> filename_buffer(filename_size + 1);
  snprintf(filename_buffer.data(),
           filename_size + 1,
           file_template.u8string().c_str(),
           frame);
  std::string filename(filename_buffer.begin(), filename_buffer.end() - 1);

  ++frame;

  GM_DBG2("SaveView",
          "Saving viewport of size " << width << "x" << height << " to file "
                                     << filename);

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

  {
    GLint current;
    glGetIntegerv(GL_DRAW_BUFFER, &current);
    glReadBuffer(current);
  }

  std::unique_ptr<FileBuffer> image =
      std::make_unique<FileBuffer>(filename,
                                   fi_image_type,
                                   width,
                                   height,
                                   bits_per_channel * number_of_channels);

  auto t1 = std::chrono::steady_clock::now();

  glReadPixels(size[0], size[1], width, height,
               use_alpha ? GL_BGRA : GL_BGR,
               channel_type, image->getBytes());

  auto t2 = std::chrono::steady_clock::now();

  saveImage(std::move(image));

  auto t3 = std::chrono::steady_clock::now();

  typedef std::chrono::duration<double, std::ratio<1>> d_seconds;
  auto dt1 = std::chrono::duration_cast<d_seconds>(t1 - t0);
  auto dt2 = std::chrono::duration_cast<d_seconds>(t2 - t1);
  auto dt3 = std::chrono::duration_cast<d_seconds>(t3 - t2);

  GM_DBG2("SaveView",
          "Captured image for saving: "
              << filename << " in " << int(1e3 * dt1.count() + 0.8) << " + "
              << int(1e3 * dt2.count() + 0.8) << " + "
              << int(1e3 * dt3.count() + 0.8) << " ms");

  render_target.pop();

  // Render offscreen buffer to active render target

  GM_DBG2("SaveView", "Render offscreen buffers to active render target");

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, render_target.getTexId());

  GLuint program_id = raster_processor.getProgramId();
  glUseProgram(program_id);
  static const GLint tex = 0;
  glUniform1iv(glGetUniformLocation(program_id, "tex"), 1, &tex);

  raster_processor.run();

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, 0);

  if (do_exit) throw gmCore::ExitException(0);
}

void SaveView::setFile(std::filesystem::path file) {

  if (file.extension() == ".png") {

    _impl->alpha_support = true;
    _impl->float_support = false;
    _impl->file_template = file;
    _impl->fi_format = FIF_PNG;
    _impl->fi_options = PNG_Z_BEST_SPEED;

    return;

  } else if (file.extension() == ".jpg" ||
             file.extension() == ".jpeg" ) {

    _impl->alpha_support = false;
    _impl->float_support = false;
    _impl->file_template = file;
    _impl->fi_format = FIF_JPEG;
    _impl->fi_options = JPEG_QUALITYGOOD;

    return;

  } else if (file.extension() == ".exr") {

    _impl->alpha_support = false;
    _impl->float_support = true;
    _impl->file_template = file;
    _impl->fi_format = FIF_EXR;
    _impl->fi_options = EXR_DEFAULT;

    return;

  } else if (file.extension() == ".tif" ||
             file.extension() == ".tiff") {

    _impl->alpha_support = true;
    _impl->float_support = true;
    _impl->file_template = file;
    _impl->fi_format = FIF_TIFF;
    _impl->fi_options = TIFF_DEFAULT;

    return;

  } else if (file.extension() == ".pfm") {

    _impl->alpha_support = false;
    _impl->float_support = true;
    _impl->file_template = file;
    _impl->fi_format = FIF_PFM;
    _impl->fi_options = 0;

    return;

  } else if (file.extension() == ".bmp") {

    _impl->alpha_support = false;
    _impl->float_support = false;
    _impl->file_template = file;
    _impl->fi_format = FIF_BMP;
    _impl->fi_options = BMP_SAVE_RLE;

    return;
  }

  GM_WRN("SaveView", "Missing suffix for a supported file format ('" << file << "') - adding '.png'");

  // Could not find supported suffix - add suffix and call again
  setFile(GM_STR(file << ".png"));
}

void SaveView::setResolution(gmCore::size2 res) {
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

void SaveView::setExit(bool on) {
  _impl->do_exit = on;
}

void SaveView::addView(std::shared_ptr<View> view) {
  if (!view) throw gmCore::InvalidArgument("null not allowed");
  _impl->views.push_back(view);
}

void SaveView::Impl::saveImage(std::unique_ptr<FileBuffer> image) {
  std::lock_guard<std::mutex> guard(save_lock);
  assert(!save_image);
  save_image = std::move(image);
  save_condition.notify_all();
}

void SaveView::Impl::save_process() {
  std::unique_lock<std::mutex> guard(save_lock);
  while(save_process_alive) {
    save_condition.wait_for(guard, std::chrono::seconds(1));

    if (!save_image) continue;

    try {
      auto path = gmCore::FileResolver::getDefault()->resolve(
          save_image->filename, gmCore::FileResolver::Check::WritableFile);
      save_image->filename = path.u8string();
    } catch (gmCore::InvalidArgument &err) {
      GM_ERR("SaveView", err.what);
      save_image.reset();
      continue;
    }

    auto t0 = std::chrono::steady_clock::now();

    bool success = FreeImage_Save(
        fi_format, save_image->bitmap, save_image->filename.data(), fi_options);

    auto t1 = std::chrono::steady_clock::now();

    typedef std::chrono::duration<double, std::ratio<1>> d_seconds;
    auto dt1 = std::chrono::duration_cast<d_seconds>(t1 - t0);

    if (success) {
      GM_DBG2("SaveView",
              "Asynchroneous image save '" << save_image->filename << "' in "
                                           << int(1e3 * dt1.count() + 0.8)
                                           << " ms");
    } else {
      GM_ERR("SaveView", "Could not save image '" << save_image->filename << "'");
    }

    save_image.reset();
  }
}

void SaveView::traverse(Visitor *visitor) {
  for (auto &v : _impl->views) v->accept(visitor);
}

END_NAMESPACE_GMGRAPHICS;

#endif
