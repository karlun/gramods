
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

  /**
     Enable or disable initialization of SDL video engine. Default is
     false.

     \gmXmlTag{gmCore,SdlContext,useVideo}
  */
  void setUseVideo(bool on) { use_video = on; }

  /**
     Enable or disable initialization of SDL audio engine. Default is
     false.

     \gmXmlTag{gmCore,SdlContext,useAudio}
  */
  void setUseAudio(bool on) { use_audio = on; }

  GM_OFI_DECLARE;

  /**
     Returns true if context has been initialized with video support.
  */
  static bool hasVideo();

  /**
     Returns true if context has been initialized with audio support.
  */
  static bool hasAudio();

  void initialize() override;

  /**
     Returns the default key, in Configuration, for the
     Object.
  */
  virtual std::string getDefaultKey() override { return "context"; }

private:

  static bool has_video;
  static bool has_audio;
  static bool has_instance;

  bool use_video = false;
  bool use_audio = false;
  
};


END_NAMESPACE_GMCORE;

#endif
#endif
