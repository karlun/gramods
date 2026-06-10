

#ifndef GRAMODS_GRAPHICS_CHROMAKEYTEXTURE
#define GRAMODS_GRAPHICS_CHROMAKEYTEXTURE

// Required before gmCore/OFactory.hh for some compilers
#include <gmCore/io_float.hh>
#include <gmCore/io_eigen.hh>

#include <gmGraphics/Texture.hh>

#include <gmCore/OFactory.hh>

#include <memory>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   The ChromaKeyTexture filters another texture making it transparent
   where the chroma key is matched within the CbCr color space within
   a specified tolerance.
*/
class ChromaKeyTexture
  : public gmGraphics::Texture {

public:

  ChromaKeyTexture();

  /**
     @see TextureInterface::updateTexture
  */
  std::optional<TextureData> updateTexture(size_t frame_number,
                                           Eye eye) override;

  /**
     Sets the texture to decode.
  */
  void setTexture(std::shared_ptr<TextureInterface> texture);

  /**
     Set the RGB key to mask with. Default is [0, 1, 0].

     \gmXmlTag{gmGraphics,ChromaKeyTexture,key}
  */
  void setKey(gmCore::float3 key);

  /**
     Set the minimum and maximum tolerances against the key when
     masking. This will specify a ramp in decimal CrCb (2D) color
     space. Default is [0.48, 0.50].

     \gmXmlTag{gmGraphics,ChromaKeyTexture,key}
  */
  void setTolerance(gmCore::float2 tol);

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
