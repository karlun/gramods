
#ifndef GRAMODS_GRAPHICS_MIXINGTEXTURE
#define GRAMODS_GRAPHICS_MIXINGTEXTURE

#include <gmCore/OFactory.hh>

#include <gmGraphics/Geometry.hh>
#include <gmGraphics/Texture.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   This texture mixes the graphics from two or more other textures,
   depending on type of mixing.
*/
class MixingTexture : public Texture {

public:
  MixingTexture();
  virtual ~MixingTexture();

  /**
     @see Texture::updateTexture
  */
  std::optional<TextureData> updateTexture(size_t frame_number,
                                           Eye eye) override;

  /**
     Propagates the specified visitor.

     @see Object::Visitor
  */
  void traverse(Visitor *visitor) override;

  /**
     Set type of mixing. Default is average.

     \gmXmlTag{gmGraphics,MixingTexture,mixType}

     @see MixingShaders
  */
  void setMixType(std::string);

  /**
     Add a texture to mix.

     \gmXmlTag{gmGraphics,MixingTexture,texture}
  */
  void addTexture(std::shared_ptr<TextureInterface>);

  GM_OFI_DECLARE;

private:
  struct Impl;
  std::unique_ptr<Impl> _impl;
};

END_NAMESPACE_GMGRAPHICS;

#endif
