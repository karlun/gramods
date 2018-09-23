
#include <gmGraphics/SdlWindow.hh>
#include <globjects/globjects.h>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE_SUB(SdlWindow, Window);
GM_OFI_PARAM(SdlWindow, GLMajor, int, SdlWindow::setGLMajor);
GM_OFI_PARAM(SdlWindow, GLMinor, int, SdlWindow::setGLMinor);
GM_OFI_POINTER(SdlWindow, context, gmCore::SdlContext, SdlWindow::setContext);

SdlWindow::SdlWindow()
  : alive(false),
    window(nullptr),
    gl_context(0) {}

SdlWindow::~SdlWindow() {
  if (gl_context) SDL_GL_DeleteContext(gl_context);
  if (window) SDL_DestroyWindow(window);
  gl_context = nullptr;
  window = nullptr;
}


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
  if (!window)
    throw std::runtime_error("Cannot create SDL window");

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, gl_major);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, gl_minor);
  if (gl_profile == "core" || gl_profile == "CORE")
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  else if (gl_profile == "es" || gl_profile == "ES")
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
  else if (gl_profile == "compatibility" || gl_profile == "COMPATIBILITY")
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
  else if (gl_profile.size() > 0)
    GM_WRN("SdlWindow", "Unknown GL profile '" << gl_profile << "' - using default");
  GM_INF("SdlWindow", "Requesting GL context " << gl_major << "." << gl_minor << " " << gl_profile);
  
  gl_context = SDL_GL_CreateContext(window);
  if (!gl_context)
    throw std::runtime_error("Cannot create GL context");

  int real_major, real_minor, real_profile;
  SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &real_major);
  SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &real_minor);
  SDL_GL_GetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, &real_profile);
  std::string str_profile =
    real_profile == SDL_GL_CONTEXT_PROFILE_CORE ? "core" :
    real_profile == SDL_GL_CONTEXT_PROFILE_ES ? "ES" :
    real_profile == SDL_GL_CONTEXT_PROFILE_COMPATIBILITY ? "compatibility" :
    "unknown";
  GM_INF("SdlWindow", "Got GL context " << real_major << "." << real_minor << " " << str_profile);
  
  globjects::init();

  alive = true;

  Window::initialize();
}

void SdlWindow::processEvents() {
  SDL_Event event;
  while(SDL_PollEvent(&event)) {
    handleEvent(event);
  }
}

bool SdlWindow::isOpen() {
  return alive;
}

void SdlWindow::swap() {
  SDL_GL_SwapWindow(window);
}

bool SdlWindow::handleEvent(SDL_Event& event) {
  switch (event.type) {

  case SDL_WINDOWEVENT:
    switch (event.window.event) {

    case SDL_WINDOWEVENT_CLOSE:
      alive = false;
      return true;

    default:
      return false;
    }

  case SDL_KEYUP:

    switch (event.key.keysym.sym) {

    case SDLK_ESCAPE:
      alive = false;
      return true;

    case SDLK_F11:
      if (SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN_DESKTOP)
        SDL_SetWindowFullscreen(window, 0);
      else
        SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
      return true;

    }

    return false;

  case SDL_QUIT:
    alive = false;
    return true;

  default:
    return false;
  }
}

END_NAMESPACE_GMGRAPHICS;
