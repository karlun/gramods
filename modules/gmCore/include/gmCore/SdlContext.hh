
#ifndef GRAMODS_CORE_SDLCONTEXT
#define GRAMODS_CORE_SDLCONTEXT

#include <gmCore/config.hh>

#ifdef gramods_ENABLE_SDL2

#include <SDL.h>

#include <gmCore/Object.hh>
#include <gmCore/OFactory.hh>

BEGIN_NAMESPACE_GMCORE;

/**
   The SdlContext class initializes SDL with the specified
   parameters. SDL will be closed when the object is destroyed.
*/
class SdlContext
  : public gmCore::Object {

public:

  SdlContext();
  ~SdlContext();

  void setGLMajor(int v) { gl_major = v; }
  void setGLMinor(int v) { gl_minor = v; }
  void setUseVideo(bool on) { use_video = on; }
  void setUseAudio(bool on) { use_audio = on; }

  GM_OFI_DECLARE(SdlContext);

  /**
     Returns true if context has been initialized with video support.
  */
  static bool hasVideo() { return has_video; }

  /**
     Returns true if context has been initialized with audio support.
  */
  static bool hasAudio() { return has_audio; }

  void initialize();

private:

  static bool has_video;
  static bool has_audio;
  static bool has_instance;

  int gl_major = 4;
  int gl_minor = 0;
  bool use_video = false;
  bool use_audio = false;
  
};


END_NAMESPACE_GMCORE;

#endif
#endif
