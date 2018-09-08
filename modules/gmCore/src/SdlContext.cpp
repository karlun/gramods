
#include <gmCore/SdlContext.hh>

BEGIN_NAMESPACE_GMCORE;

GM_OFI_DEFINE(SdlContext);
GM_OFI_PARAM(SdlContext, GLMajor, int, SdlContext::setGLMajor);
GM_OFI_PARAM(SdlContext, GLMinor, int, SdlContext::setGLMinor);
GM_OFI_PARAM(SdlContext, useVideo, bool, SdlContext::setUseVideo);
GM_OFI_PARAM(SdlContext, useAudio, bool, SdlContext::setUseAudio);

bool SdlContext::has_audio = false;
bool SdlContext::has_video = false;
bool SdlContext::has_instance = false;

SdlContext::SdlContext() {
  if (has_instance)
    throw std::runtime_error("Cannot instantiate SdlContext - SDL already initialized");
  has_instance = true;
}

SdlContext::~SdlContext() {
  has_instance = false;
  SDL_Quit();
}


void SdlContext::initialize() {
  if (isInitialized()) return;
  Object::initialize();

  Uint32 sdl_flags = 0;

  if (use_audio)
    sdl_flags |= SDL_INIT_AUDIO;

  if (use_video) {
    sdl_flags |= SDL_INIT_VIDEO;
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, gl_major);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, gl_minor);
  }

  if (sdl_flags == 0)
    return;

  if (SDL_Init(sdl_flags) < 0)
    throw std::runtime_error("Cannot initialize SDL");

  has_audio = use_audio;
  has_video = use_video;
}

END_NAMESPACE_GMCORE;
