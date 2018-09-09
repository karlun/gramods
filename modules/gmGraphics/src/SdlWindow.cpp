
#include <gmGraphics/SdlWindow.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE_SUB(SdlWindow, Window);
GM_OFI_POINTER(SdlWindow, context, gmCore::SdlContext, SdlWindow::setContext);

SdlWindow::SdlWindow() {}
SdlWindow::~SdlWindow() {}


void SdlWindow::makeGLContextCurrent() {
  SDL_GL_MakeCurrent(window, gl_context);
}

void SdlWindow::initialize() {
  if (!gmCore::SdlContext::hasVideo())
    throw std::runtime_error("Cannot open SDL window - no SDL video context initialized");

  int video_flags = SDL_WINDOW_OPENGL;
  if (fullscreen)
    video_flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
  video_flags |= SDL_WINDOW_RESIZABLE;

  window = SDL_CreateWindow(title.c_str(),
                            SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                            size[0], size[1], video_flags);
  gl_context = SDL_GL_CreateContext(window);
}

END_NAMESPACE_GMGRAPHICS;
