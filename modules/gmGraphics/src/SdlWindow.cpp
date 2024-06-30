
#include <gmGraphics/SdlWindow.hh>

#ifdef gramods_ENABLE_SDL2

#include <gmCore/Stringify.hh>

#include <GL/glew.h>
#include <GL/gl.h>

#include <limits>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE_SUB(SdlWindow, Window);
GM_OFI_PARAM2(SdlWindow, useQuadBuffers, bool, setUseQuadBuffers);
GM_OFI_PARAM2(SdlWindow, GLMajor, int, setGLMajor);
GM_OFI_PARAM2(SdlWindow, GLMinor, int, setGLMinor);
GM_OFI_PARAM2(SdlWindow, GLProfile, std::string, setGLProfile);
GM_OFI_POINTER2(SdlWindow, context, gmCore::SdlContext, setContext);

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

void GLAPIENTRY MessageCallback
(GLenum,
 GLenum type,
 GLuint,
 GLenum severity,
 GLsizei,
 const GLchar* message,
 const void*) {
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
    GM_DBG1("OpenGL", "OpenGL " << type_str
           << ", severity = " << severity_str
           << ", message = " << message );
  }
}

void SdlWindow::initialize() {
  if (!gmCore::SdlContext::hasVideo())
    throw std::runtime_error("Cannot open SDL window - no SDL video context initialized");

  SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

  if (gl_use_quad_buffers) SDL_GL_SetAttribute(SDL_GL_STEREO, 1);
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

  if (!SDL_SetHint(SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS, "0"))
    GM_WRN("SdlWindow", "Could not suggest SDL to stay fullscreen even with focus loss");

  int video_flags = SDL_WINDOW_OPENGL;
  if (fullscreen)
    video_flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
  video_flags |= SDL_WINDOW_RESIZABLE;

  int display_count = SDL_GetNumVideoDisplays();
  if (int(display) >= display_count) {
    GM_ERR("SdlWindow", "requested display " << display << " is not available (" << display_count << " available).");
    throw gmCore::InvalidArgument(GM_STR("requested display " << display << " is not available (" << display_count << " available)."));
  }

  SDL_Rect display_bounds;
  if (SDL_GetDisplayBounds(display, &display_bounds)) {
    GM_ERR("SdlWindow", SDL_GetError());
    throw std::runtime_error("Could not get display bounds");
  }

  int win_pos_x = position[0] == std::numeric_limits<int>::max() ?
    SDL_WINDOWPOS_UNDEFINED_DISPLAY(display) : display_bounds.x + position[0];
  int win_pos_y = position[1] == std::numeric_limits<int>::max() ?
    SDL_WINDOWPOS_UNDEFINED_DISPLAY(display) : display_bounds.y + position[1];

  GM_DBG1("SdlWindow",
         "Requesting window (" << title << ") "
         << size[0] << "x" << size[1]
         << " @" << win_pos_x << "," << win_pos_y
         << (fullscreen?" fullscreen":"") << " (display " << display << ")");
  window = SDL_CreateWindow(title.c_str(),
                            win_pos_x, win_pos_y,
                            size[0], size[1], video_flags);
  if (!window) {
    GM_ERR("SdlWindow", SDL_GetError());
    throw std::runtime_error("Could not create SDL window");
  }

  sdl_windows[SDL_GetWindowID(window)] = std::static_pointer_cast<SdlWindow>(shared_from_this());

  GM_DBG1("SdlWindow", "Requesting GL context " << gl_major << "." << gl_minor << " " << gl_profile);
  gl_context = SDL_GL_CreateContext(window);
  if (!gl_context) {
    GM_ERR("SdlWindow", SDL_GetError());
    throw std::runtime_error("Could not create GL context");
  }

  int real_major, real_minor, real_profile;
  SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &real_major);
  SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &real_minor);
  SDL_GL_GetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, &real_profile);
  std::string str_profile =
    real_profile == SDL_GL_CONTEXT_PROFILE_CORE ? "core" :
    real_profile == SDL_GL_CONTEXT_PROFILE_ES ? "ES" :
    real_profile == SDL_GL_CONTEXT_PROFILE_COMPATIBILITY ? "compatibility" :
    "unknown";
  GM_DBG1("SdlWindow", "Got GL context " << real_major << "." << real_minor << " " << str_profile);

  GLenum err = glewInit();
  if (GLEW_OK != err) {
    throw std::runtime_error(GM_STR("Cannot link GL symbols: " << glewGetErrorString(err)));
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
      if (_that) _that->processEvent(event);
    }
      break;
    case SDL_KEYUP:
    case SDL_KEYDOWN: {
      auto _that = sdl_windows[event.key.windowID].lock();
      if (_that) _that->processEvent(event);
    }
      break;
    case SDL_QUIT: {
      auto sdl_windows_copy = sdl_windows;
      for (auto it : sdl_windows_copy) {
        auto _that = it.second.lock();
        if (_that) _that->close();
      }
    }
    }
  }
}

bool SdlWindow::isOpen() {
  return alive;
}

void SdlWindow::swap() {
  SDL_GL_MakeCurrent(window, gl_context);
  SDL_GL_SwapWindow(window);
}

void SdlWindow::close() {
  GM_DBG1("SdlWindow", "Closing window " << title);

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

gmCore::size2 SdlWindow::getSize() {
  if (!window) return gmCore::size2 { 0, 0 };
  int width, height;
  SDL_GetWindowSize(window, &width, &height);
  return gmCore::size2 { (size_t)width, (size_t)height };
}

bool SdlWindow::processEvent(SDL_Event& event) {
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

    {
      key_event evt(true, (int)event.key.keysym.sym);
      return handleEvent(&evt);
    }

  case SDL_KEYDOWN:

    {
      key_event evt(false, (int)event.key.keysym.sym);
      return handleEvent(&evt);
    }

  default:
    return false;
  }
}

void SdlWindow::traverse(Visitor *visitor) {
  Window::traverse(visitor);
  if (context) context->accept(visitor);
}

END_NAMESPACE_GMGRAPHICS;

#endif
