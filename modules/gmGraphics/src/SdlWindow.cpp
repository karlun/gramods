
#include <gmGraphics/SdlWindow.hh>

#include <GL/glew.h>
#include <GL/gl.h>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE_SUB(SdlWindow, Window);
GM_OFI_PARAM(SdlWindow, GLMajor, int, SdlWindow::setGLMajor);
GM_OFI_PARAM(SdlWindow, GLMinor, int, SdlWindow::setGLMinor);
GM_OFI_POINTER(SdlWindow, context, gmCore::SdlContext, SdlWindow::setContext);

std::map<unsigned int, std::weak_ptr<SdlWindow>> SdlWindow::sdl_windows;

SdlWindow::SdlWindow()
  : alive(false),
    window(nullptr),
    gl_context(0) {}

SdlWindow::~SdlWindow() {
  close();
}


void SdlWindow::makeGLContextCurrent() {
  SDL_GL_MakeCurrent(window, gl_context);
}

void MessageCallback(GLenum source,
                     GLenum type,
                     GLuint id,
                     GLenum severity,
                     GLsizei length,
                     const GLchar* message,
                     const void* userParam) {
  std::string type_str;
  switch(type) {
  case GL_DEBUG_TYPE_ERROR: type_str = "error"; break;
  case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: type_str = "deprecated behavior"; break;
  case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: type_str = "undefined behavior"; break;
  case GL_DEBUG_TYPE_PORTABILITY: type_str = "portability problem"; break;
  case GL_DEBUG_TYPE_PERFORMANCE: type_str = "performance issue"; break;
  case GL_DEBUG_TYPE_MARKER: type_str = "marker"; break;
  case GL_DEBUG_TYPE_PUSH_GROUP: type_str = "push"; break;
  case GL_DEBUG_TYPE_POP_GROUP: type_str = "pop"; break;
  case GL_DEBUG_TYPE_OTHER: type_str = "other"; break;
  default:
    type_str = "unknown";
  }

  std::string severity_str;
  switch(severity) {
  case GL_DEBUG_SEVERITY_HIGH: severity_str = "high"; break;
  case GL_DEBUG_SEVERITY_MEDIUM: severity_str = "medium"; break;
  case GL_DEBUG_SEVERITY_LOW: severity_str = "low"; break;
  case GL_DEBUG_SEVERITY_NOTIFICATION: severity_str = "notification"; break;
  default:
    severity_str = "unknown";
  }

  switch(severity) {
  case GL_DEBUG_SEVERITY_HIGH:
    GM_ERR("OpenGL", "OpenGL " << type_str
           << ", severity = " << severity_str
           << ", message = " << message );
    break;
  case GL_DEBUG_SEVERITY_MEDIUM:
    GM_WRN("OpenGL", "OpenGL " << type_str
           << ", severity = " << severity_str
           << ", message = " << message );
    break;
  case GL_DEBUG_SEVERITY_LOW:
  case GL_DEBUG_SEVERITY_NOTIFICATION:
    GM_INF("OpenGL", "OpenGL " << type_str
           << ", severity = " << severity_str
           << ", message = " << message );
  }
}

void SdlWindow::initialize() {
  if (!gmCore::SdlContext::hasVideo())
    throw std::runtime_error("Cannot open SDL window - no SDL video context initialized");

  int video_flags = SDL_WINDOW_OPENGL;
  if (fullscreen)
    video_flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
  video_flags |= SDL_WINDOW_RESIZABLE;

  GM_INF("SdlWindow", "Requesting window (" << title << ") " << size[0] << "x" << size[1] << " " << (fullscreen?"fullscreen":""));
  window = SDL_CreateWindow(title.c_str(),
                            SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                            size[0], size[1], video_flags);
  if (!window)
    throw std::runtime_error("Cannot create SDL window");
  sdl_windows[SDL_GetWindowID(window)] = std::static_pointer_cast<SdlWindow>(shared_from_this());

  if (gl_major > 0) SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, gl_major);
  if (gl_minor > 0) SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, gl_minor);
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

  GLenum err = glewInit();
  if (GLEW_OK != err) {
    std::stringstream s;
    s << "Cannot link GL symbols: " << glewGetErrorString( err );
    throw std::runtime_error(s.str());
  }

  glEnable              ( GL_DEBUG_OUTPUT );
  glDebugMessageCallback( MessageCallback, 0 );
  
  alive = true;

  Window::initialize();
}

void SdlWindow::processEvents() {
  SDL_Event event;
  while(SDL_PollEvent(&event)) {
    switch (event.type) {
    case SDL_WINDOWEVENT: {
      auto _that = sdl_windows[event.window.windowID].lock();
      if (_that) _that->handleEvent(event);
    }
      break;
    case SDL_KEYUP: {
      auto _that = sdl_windows[event.key.windowID].lock();
      if (_that) _that->handleEvent(event);
    }
      break;
    case SDL_QUIT:
      for (auto it : sdl_windows) {
        auto _that = it.second.lock();
        if (_that) _that->close();
      }
    }
  }
}

bool SdlWindow::isOpen() {
  return alive;
}

void SdlWindow::swap() {
  SDL_GL_SwapWindow(window);
}

void SdlWindow::close() {
  GM_INF("SdlWindow", "Closing window " << title);

  alive = false;

  if (gl_context){
    SDL_GL_DeleteContext(gl_context);
    gl_context = nullptr;
  }

  if (window) {
    sdl_windows.erase(SDL_GetWindowID(window));
    SDL_DestroyWindow(window);
    window = nullptr;
  }
}

gmTypes::size2 SdlWindow::getSize() {
  if (!window) return gmTypes::size2 { 0, 0 };
  int width, height;
  SDL_GetWindowSize(window, &width, &height);
  return gmTypes::size2 { (size_t)width, (size_t)height };
}

bool SdlWindow::handleEvent(SDL_Event& event) {
  switch (event.type) {

  case SDL_WINDOWEVENT:

    switch (event.window.event) {

    case SDL_WINDOWEVENT_CLOSE:
      close();
      return true;

    default:
      return false;
    }

  case SDL_KEYUP:

    switch (event.key.keysym.sym) {

    case SDLK_ESCAPE:
      close();
      return true;

    case SDLK_F11:
      if (SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN_DESKTOP)
        SDL_SetWindowFullscreen(window, 0);
      else
        SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
      return true;

    }

    return false;

  default:
    return false;
  }
}

END_NAMESPACE_GMGRAPHICS;
