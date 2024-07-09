

#ifndef GRAMODS_GRAPHICS_MULTISCOPICTEXTURESPLITTER
#define GRAMODS_GRAPHICS_MULTISCOPICTEXTURESPLITTER

// Required before gmCore/OFactory.hh for some compilers
#include <gmCore/io_size.hh>

#include <gmGraphics/Texture.hh>

#include <gmCore/OFactory.hh>

#include <memory>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   Texture that reads off one part at a time from another texture that
   encodes multiple textures, typically for stereoscopic transcoding.
*/
class MultiscopicTextureSplitter
  : public gmGraphics::Texture {

public:

  MultiscopicTextureSplitter();

  /**
     Updates the texture and returns the ID of the associated GL
     texture object. Must be called with GL context. This method will
     perform off-screen rendering.

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
     Set the texture to split.

     \gmXmlTag{gmGraphics,MultiscopicTextureSplitter,texture}
  */
  void setTexture(std::shared_ptr<TextureInterface> texture);

  /**
     Sets the type of multiscopic splitting of the input
     texture. Possible values are

      - 0, horizontal (default) and
      - 1, vertical

     \gmXmlTag{gmGraphics,MultiscopicTextureSplitter,splitType}
  */
  void setSplitType(size_t type);

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
