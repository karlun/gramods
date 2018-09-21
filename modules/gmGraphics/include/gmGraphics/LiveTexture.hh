
#ifndef GRAMODS_GRAPHICS_LIVETEXTURE
#define GRAMODS_GRAPHICS_LIVETEXTURE

#include <gmGraphics/config.hh>

#include <gmCore/Object.hh>
#include <globjects/Texture.h>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   The base of globjects::Texture classes that should be dynamically
   updated.
*/
class LiveTexture
  : public gmCore::Object {

public:

  /**
     Updates the live texture with new data.
  */
  virtual void update() = 0;

  /**
     Returns a raw pointer to the internal texture object.
  */
  virtual globjects::Texture * getTexture() = 0;

};

END_NAMESPACE_GMGRAPHICS;

#endif
