

#ifndef GRAMODS_GRAPHICS_YUVDECODETEXTURE
#define GRAMODS_GRAPHICS_YUVDECODETEXTURE

#include <gmGraphics/Texture.hh>

#include <gmCore/OFactory.hh>
#include <gmTypes/float.hh>
#include <gmTypes/eigen.hh>

#include <memory>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   Decoder of Yuv encoded texture into a new texture.
*/
class YuvDecodeTexture
  : public gmGraphics::Texture {

public:

  YuvDecodeTexture();

  /**
     Updates the texture with new data. Must be called with GL
     context.
  */
  void update();

  /**
     Returns the ID of the associated GL texture object.
  */
  GLuint getGLTextureID();

  /**
     Sets the texture to decode.
  */
  void setTexture(std::shared_ptr<Texture> texture);

  /**
     Sets the conversion matrix from YUV space to RGB. Default is
     BT.601.

     \gmXmlTag{gmGraphics,YuvDecodeTexture,yuvToRgb}

     \sa gramods::operator>>(std::istream &, Eigen::Matrix3f &)
  */
  void setYuvToRgb(Eigen::Matrix3f m);

  /**
     Sets the range of the UV values. Default is BT.601.

     \gmXmlTag{gmGraphics,YuvDecodeTexture,uvRange}
  */
  void setUvRange(gmTypes::float2 v);

  GM_OFI_DECLARE;

private:

  struct Impl;
  std::unique_ptr<Impl> _impl;
};

END_NAMESPACE_GMGRAPHICS;

#endif
