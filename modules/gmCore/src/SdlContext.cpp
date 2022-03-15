
#include <gmCore/SdlContext.hh>

#ifdef gramods_ENABLE_SDL2

BEGIN_NAMESPACE_GMCORE;

GM_OFI_DEFINE(SdlContext);
GM_OFI_PARAM2(SdlContext, useVideo, bool, setUseVideo);
GM_OFI_PARAM2(SdlContext, useAudio, bool, setUseAudio);

bool SdlContext::has_audio = false;
bool SdlContext::has_video = false;
bool SdlContext::has_instance = false;

bool SdlContext::hasVideo() { return has_video; }
bool SdlContext::hasAudio() { return has_audio; }

SdlContext::SdlContext() {
  if (has_instance)
    throw RuntimeException("Cannot instantiate SdlContext - SDL already initialized");
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
  }

  if (sdl_flags == 0)
    return;

  if (SDL_Init(sdl_flags) < 0)
    throw std::runtime_error("Cannot initialize SDL");

  has_audio = use_audio;
  has_video = use_video;
}

END_NAMESPACE_GMCORE;
#endif
