
#ifndef GRAMODS_GRAPHICS_TEXTUREINTERFACE
#define GRAMODS_GRAPHICS_TEXTUREINTERFACE

#include <gmGraphics/config.hh>

#include <GL/glew.h>
#include <GL/gl.h>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   The interface for classes providing texture data for rendering.
*/
class TextureInterface {

public:

  /**
     Updates the texture with new data. Must be called with GL
     context.
  */
  virtual void update() = 0;

  /**
     Returns the ID of the associated GL texture object.
  */
  virtual GLuint getGLTextureID() = 0;

};

END_NAMESPACE_GMGRAPHICS;

#endif
