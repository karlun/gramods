

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
   Decoder of Yuv encoded texture into a new texture.
*/
class ChromaKeyTexture
  : public gmGraphics::Texture {

public:

  ChromaKeyTexture();

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
