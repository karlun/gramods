

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
     @see TextureInterface::updateTexture
  */
  std::optional<TextureData> updateTexture(size_t frame_number,
                                           Eye eye) override;

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
