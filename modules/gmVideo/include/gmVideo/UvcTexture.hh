
#ifndef GRAMODS_VIDEO_UVCTEXTURE
#define GRAMODS_VIDEO_UVCTEXTURE

#include <gmVideo/config.hh>

#ifdef gramods_ENABLE_libuvc

#include <globjects/Texture.h>
#include <memory>

BEGIN_NAMESPACE_GMVIDEO;

/**
   The UvcTexture connects to a USB video class device and populates
   its texture data with RGB streamed from that device.
*/
class UvcTexture
  : public globjects::Texture {

public:

  /**
     Connects to the first available UVC device matching the specified
     ID or serial number. Throws std::runtime_exception if streaming
     could not be started for some reason.
  */
  UvcTexture(int vendor = 0, int product = 0, std::string serial = "");

  ~UvcTexture();

private:

  class _UvcTexture;
  std::unique_ptr<_UvcTexture> _this;

};

END_NAMESPACE_GMVIDEO;

#endif
#endif
