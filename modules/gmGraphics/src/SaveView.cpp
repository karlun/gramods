
#include <gmGraphics/SaveView.hh>

#ifdef gramods_ENABLE_FreeImage

#include <gmGraphics/FreeImage.hh>

#include <FreeImage.h>

#include <GL/glew.h>
#include <GL/gl.h>

#include <chrono>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE_SUB(SaveView, View);
GM_OFI_PARAM(SaveView, file, std::string, SaveView::setFile);
GM_OFI_POINTER(SaveView, view, gmGraphics::View, SaveView::setView);

struct SaveView::Impl {
  std::string file_template;
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
    static bool message_shown = false;
    if (!message_shown) {
      GM_ERR("SaveView", "Could not save - empty filename");
      message_shown = true;
    }
    return;
  }

  auto t0 = std::chrono::steady_clock::now();

  GLint size[4];
  glGetIntegerv( GL_VIEWPORT, size );

  int width = size[2]-size[0];
  int height = size[3]-size[1];
  int image_size = 4*width*height;

  FIBITMAP *bitmap = FreeImage_Allocate(width, height, 32);
  BYTE * bytes = FreeImage_GetBits(bitmap);

  glReadBuffer(GL_BACK);
  glReadPixels(size[0], size[1], width, height,
               GL_BGRA, GL_UNSIGNED_BYTE, bytes);

  auto t1 = std::chrono::steady_clock::now();

  char filename[1024];
  snprintf(filename, 1023, file_template.c_str(), frame++);

  FreeImage_Save(FIF_PNG, bitmap, filename, PNG_Z_BEST_SPEED);
  FreeImage_Unload(bitmap);

  auto t2 = std::chrono::steady_clock::now();

  typedef std::chrono::duration<double, std::ratio<1>> d_seconds;
  auto dt1 = std::chrono::duration_cast<d_seconds>(t1 - t0);
  auto dt2 = std::chrono::duration_cast<d_seconds>(t2 - t1);

  GM_VINF("SaveView", "Captured and saved image " << filename << " in " << int(1e3 * dt1.count() + 0.8) << " + " << int(1e3 * dt2.count() + 0.8) << " ms");
}

void SaveView::setFile(std::string file) {
  if (file.rfind(".png") == file.size() - 4) {
    _impl->file_template = file;
    return;
  }

  _impl->file_template = static_cast<std::stringstream&>
    (std::stringstream() << file << ".png").str();
}

void SaveView::setView(std::shared_ptr<View> view) {
  _impl->view = view;
}

END_NAMESPACE_GMGRAPHICS;

#endif
