

#ifndef GRAMODS_GRAPHICS_MULTISCOPICTEXTUREMULTIPLEXER
#define GRAMODS_GRAPHICS_MULTISCOPICTEXTUREMULTIPLEXER

#include <gmGraphics/Texture.hh>

#include <gmCore/OFactory.hh>
#include <gmCore/io_float.hh>
#include <gmCore/io_eigen.hh>

#include <memory>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   Texture that returns one other texture at a time that encodes
   multiple textures, typically for stereoscopic transcoding.
*/
class MultiscopicTextureMultiplexer
  : public gmGraphics::Texture {

public:

  MultiscopicTextureMultiplexer();

  /**
     @see TextureInterface::updateTexture
  */
  std::optional<TextureData> updateTexture(size_t frame_number,
                                           Eye eye) override;

  /**
     Adds a texture to the set of multiscopic textures.
  */
  void addTexture(std::shared_ptr<TextureInterface> texture);

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
