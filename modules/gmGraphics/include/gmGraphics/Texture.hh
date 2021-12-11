

#ifndef GRAMODS_GRAPHICS_TEXTURE
#define GRAMODS_GRAPHICS_TEXTURE

#include <gmGraphics/config.hh>

#include <gmCore/Object.hh>
#include <gmGraphics/TextureInterface.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   The base of classes providing texture data for rendering.
*/
class Texture
  : public gmCore::Object,
    public TextureInterface {

public:

  /**
     Returns the default key, in Configuration, for the
     Object.
  */
  virtual std::string getDefaultKey() override { return "texture"; }

};

END_NAMESPACE_GMGRAPHICS;

#endif
