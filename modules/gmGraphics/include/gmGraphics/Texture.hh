
#ifndef GRAMODS_GRAPHICS_TEXTURE
#define GRAMODS_GRAPHICS_TEXTURE

#include <gmGraphics/config.hh>

#include <gmCore/Object.hh>

#include <glbinding/gl/types.h>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   The base of classes providing texture data for rendering.
*/
class Texture
  : public gmCore::Object {

public:

  /**
     Updates the texture with new data. Must be called with GL
     context.
  */
  virtual void update() = 0;

  /**
     Returns the ID of the associated GL texture object.
  */
  virtual gl::GLuint getGLTextureID() = 0;

};

END_NAMESPACE_GMGRAPHICS;

#endif
