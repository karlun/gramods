
#include <gmGraphics/SaveView.hh>

#ifdef gramods_ENABLE_FreeImage

#include <gmGraphics/FreeImage.hh>
#include <gmCore/RunOnce.hh>

#include <FreeImage.h>

#include <GL/glew.h>
#include <GL/gl.h>

#include <chrono>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE_SUB(SaveView, View);
GM_OFI_PARAM(SaveView, file, std::string, SaveView::setFile);
GM_OFI_POINTER(SaveView, view, gmGraphics::View, SaveView::setView);

struct SaveView::Impl {
  bool alpha_support = true;
  std::string file_template = "SaveView.png";
  FREE_IMAGE_FORMAT fi_format = FIF_PNG;
  int fi_options = PNG_Z_BEST_SPEED;

  std::shared_ptr<View> view;
  int frame = 0;

  std::shared_ptr<FreeImage> free_image;

  void renderFullPipeline(ViewSettings settings);
};

SaveView::SaveView()
  : _impl(std::make_unique<Impl>()) {
  _impl->free_image = FreeImage::get();
}

void SaveView::renderFullPipeline(ViewSettings settings) {
  populateViewSettings(settings);
  _impl->renderFullPipeline(settings);
}

void SaveView::Impl::renderFullPipeline(ViewSettings settings) {
  view->renderFullPipeline(settings);

  if (file_template.size() == 0) {
    GM_RUNONCE(GM_ERR("SaveView", "Could not save - empty filename"));
    return;
  }

  auto t0 = std::chrono::steady_clock::now();

  GLint size[4];
  glGetIntegerv( GL_VIEWPORT, size );

  int width = size[2]-size[0];
  int height = size[3]-size[1];

  FIBITMAP *bitmap = alpha_support ?
    FreeImage_Allocate(width, height, 32) :
    FreeImage_Allocate(width, height, 24);
  BYTE * bytes = FreeImage_GetBits(bitmap);

  {
    GLint current;
    glGetIntegerv(GL_DRAW_BUFFER, &current);
    glReadBuffer(current);
  }

  glReadPixels(size[0], size[1], width, height,
               alpha_support ? GL_BGRA : GL_BGR,
               GL_UNSIGNED_BYTE, bytes);

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
}

void SaveView::setFile(std::string file) {

  if (file.rfind(".png") != std::string::npos &&
      file.rfind(".png") == file.size() - 4) {

    _impl->alpha_support = true;
    _impl->file_template = file;
    _impl->fi_format = FIF_PNG;
    _impl->fi_options = PNG_Z_BEST_SPEED;

    return;

  } else if ((file.rfind(".jpg") != std::string::npos &&
              file.rfind(".jpg") == file.size() - 4) ||
             (file.rfind(".jpeg") != std::string::npos &&
              file.rfind(".jpeg") == file.size() - 5)) {

    _impl->alpha_support = false;
    _impl->file_template = file;
    _impl->fi_format = FIF_JPEG;
    _impl->fi_options = JPEG_QUALITYBAD;

    return;

  } else if (file.rfind(".bmp") != std::string::npos &&
             file.rfind(".bmp") == file.size() - 4) {

    _impl->alpha_support = false;
    _impl->file_template = file;
    _impl->fi_format = FIF_BMP;
    _impl->fi_options = BMP_SAVE_RLE;

    return;
  }

  // Could not find supported suffix - add suffix and call again
  setFile(static_cast<std::stringstream&>
          (std::stringstream() << file << ".png").str());
}

void SaveView::setView(std::shared_ptr<View> view) {
  _impl->view = view;
}

END_NAMESPACE_GMGRAPHICS;

#endif
