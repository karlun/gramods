
#ifndef GRAMODS_GRAPHICS_UVCTEXTURE
#define GRAMODS_GRAPHICS_UVCTEXTURE

#include <gmGraphics/config.hh>

#ifdef gramods_ENABLE_libuvc

#include <gmTypes/all.hh>
#include <gmCore/OFactory.hh>
#include <gmGraphics/Texture.hh>
#include <memory>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   The UvcTexture connects to a USB video class device and populates
   its texture data with RGB streamed from that device.
*/
class UvcTexture
  : public gmGraphics::Texture {

public:

  UvcTexture();

  void initialize();

  /**
     Updates the live texture with new data. Must be called with GL
     context.
  */
  void update();

  /**
     Returns the ID of the associated GL texture object.
  */
  GLuint getGLTextureID();

  /**
     Set the vendor ID of the UVC device to stream from. As XML
     attribute, use the 0x prefix to enter the value in hexadecimal
     format.
  */
  void setVendor(int v) { vendor = v; }

  /**
     Set the product ID of the UVC device to stream from. As XML
     attribute, use the 0x prefix to enter the value in hexadecimal
     format.
  */
  void setProduct(int p) { product = p; }

  /**
     Set the serial ID of the UVC device to stream from.
  */
  void setSerial(std::string s) { serial = s; }

  /**
     Set the resolution to attempt to negotiate with the UVC device.
  */
  void setResolution(gmTypes::size2 res);

  /**
     Set the framerate to attempt to negotiate with the UVC device.
  */
  void setFramerate(int fps);

  /**
     Set the format to attempt to negotiate with the UVC device.

     Values recognized by UVC, but not necessarily supported by all
     devices, are any, uncompressed, compressed, yuyv, uyvy, rgb, bgr,
     mjpeg, gray8, gray16, by8, ba81, sgrbg8, sgbrg8, srggb8, sbggr8.
   */
  void setFormat(std::string fmt);

  GM_OFI_DECLARE;

private:

  class Impl;
  std::unique_ptr<Impl> _impl;

  int vendor;
  int product;
  std::string serial;

};

END_NAMESPACE_GMGRAPHICS;

#endif
#endif
