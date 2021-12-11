

#ifndef GRAMODS_GRAPHICS_CHROMAKEYTEXTURE
#define GRAMODS_GRAPHICS_CHROMAKEYTEXTURE

#include <gmGraphics/Texture.hh>

#include <gmCore/OFactory.hh>
#include <gmCore/float.hh>
#include <gmCore/eigen.hh>

#include <memory>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   Decoder of Yuv encoded texture into a new texture.
*/
class ChromaKeyTexture
  : public gmGraphics::Texture {

public:

  ChromaKeyTexture();

  /**
     Updates the texture with new data. Must be called with GL
     context.
  */
  void update();

  /**
     Returns the ID of the associated GL texture object.
  */
  GLuint getGLTextureID() override;

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

  GM_OFI_DECLARE;

private:

  struct Impl;
  std::unique_ptr<Impl> _impl;
};

END_NAMESPACE_GMGRAPHICS;

#endif
