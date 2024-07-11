
#ifndef GRAMODS_GRAPHICS_UVCTEXTURE
#define GRAMODS_GRAPHICS_UVCTEXTURE

#include <gmGraphics/config.hh>

#ifdef gramods_ENABLE_libuvc

// Required before gmCore/OFactory.hh for some compilers
#include <gmCore/io_size.hh>

#include <gmCore/OFactory.hh>
#include <gmCore/VideoSource.hh>

#include <gmGraphics/Texture.hh>
#include <memory>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   The UvcTexture connects to a USB video class device and populates
   its texture data with RGB streamed from that device.
*/
class UvcTexture :
#ifdef gramods_ENABLE_OpenCV
  public gmCore::VideoSource,
#endif
  public gmGraphics::Texture {

public:

  UvcTexture();

  void initialize() override;

  /**
     Updates the texture and returns the ID of the associated GL
     texture object. Must be called with GL context. Observe also that
     this method may perform off-screen rendering.

     @param frame_number The current frame being rendered. This number
     should increment by one for each frame, however it may wrap
     around and start over at zero.

     @param eye Eye for which the texture is requested. An
     implementation may use different textures for different eyes or
     may reuse the same texture for all eyes.

     @returns OpenGL texture ID of the updated texture
  */
  GLuint updateTexture(size_t frame_number, Eye eye) override;

  /**
     Set the vendor ID of the UVC device to stream from. As XML
     attribute, use the 0x prefix to enter the value in hexadecimal
     format.

     \gmXmlTag{gmGraphics,UvcTexture,vendor}
  */
  void setVendor(int v) { vendor = v; }

  /**
     Set the product ID of the UVC device to stream from. As XML
     attribute, use the 0x prefix to enter the value in hexadecimal
     format.

     \gmXmlTag{gmGraphics,UvcTexture,product}
  */
  void setProduct(int p) { product = p; }

  /**
     Set the serial ID of the UVC device to stream from.

     \gmXmlTag{gmGraphics,UvcTexture,serial}
  */
  void setSerial(std::string s) { serial = s; }

  /**
     Set the resolution to attempt to negotiate with the UVC device.

     \gmXmlTag{gmGraphics,UvcTexture,resolution}
  */
  void setResolution(gmCore::size2 res);

  /**
     Set the framerate to attempt to negotiate with the UVC device.

     \gmXmlTag{gmGraphics,UvcTexture,framerate}
  */
  void setFramerate(int fps);

  /**
     Set the format to attempt to negotiate with the UVC device.

     Values recognized by UVC, but not necessarily supported by all
     devices, are any, uncompressed, compressed, yuyv, uyvy, rgb, bgr,
     mjpeg, gray8, gray16, by8, ba81, sgrbg8, sgbrg8, srggb8,
     sbggr8. Default is any.

     \gmXmlTag{gmGraphics,UvcTexture,format}
  */
  void setFormat(std::string fmt);

  /**
     If this is set to false, the incoming image data will be
     downloaded to texture as is, if true they will be converted to
     8-bit RGB. Default is true.

     Only YUYV and UYVY formats can be uploaded as texture. All other
     formats need to be converted to RGB. The YUV data are encoded
     into a 4-channel texture (RGBA8) and it is up to the client code
     using this texture to decode each four channel texel into two
     three channel RGB pixels.

     \gmXmlTag{gmGraphics,UvcTexture,decode}
  */
  void setDecode(bool on);

  /**
     Trigger a still image capture (UVC STILL_IMAGE_FRAME). Returns
     false iff still image capture could not be triggered.
  */
  bool triggerStill(gmCore::size2 res);

#ifdef gramods_ENABLE_OpenCV

  /**
     Retrieve the latest read image in the video source in OpenCV
     format.

     @param[out] image The latest image in the video source.

     @returns True if the image was successfully read.
  */
  bool retrieve(cv::Mat &image) override;

#endif

  GM_OFI_DECLARE;

private:

  struct Impl;
  std::unique_ptr<Impl> _impl;

  int vendor;
  int product;
  std::string serial;
};

END_NAMESPACE_GMGRAPHICS;

#endif
#endif
