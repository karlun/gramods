

#ifndef GRAMODS_GRAPHICS_YUVDECODETEXTURE
#define GRAMODS_GRAPHICS_YUVDECODETEXTURE

// Required before gmCore/OFactory.hh for some compilers
#include <gmCore/io_float.hh>
#include <gmCore/io_eigen.hh>

#include <gmGraphics/Texture.hh>

#include <gmCore/OFactory.hh>

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
     @see TextureInterface::updateTexture
  */
  std::optional<TextureData> updateTexture(size_t frame_number,
                                           Eye eye) override;

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
  void setUvRange(gmCore::float2 v);

  /**
     Propagates the specified visitor.

     @see Object::Visitor
  */
  void traverse(Visitor *visitor) override;

  GM_OFI_DECLARE;

private:

  struct Impl;
  std::unique_ptr<Impl> _impl;
};

END_NAMESPACE_GMGRAPHICS;

#endif
