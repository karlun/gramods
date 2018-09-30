
#ifndef GRAMODS_GRAPHICS_VIDEOTEXTURE
#define GRAMODS_GRAPHICS_VIDEOTEXTURE

#include <gmGraphics/Texture.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   Texture updating its context using a video stream.
*/
class VideoTexture
  : public gmGraphics::Texture {

public:

  VideoTexture();

  void initialize();

  void setFile(std::string f) { file = f; }

  /**
     Updates the live texture with new data. Must be called with GL
     context.
  */
  void update();

  /**
     Returns a raw pointer to the internal texture object.
  */
  GLuint getGLTextureID();

  GM_OFI_DECLARE;

private:

  class Impl;
  std::unique_ptr<Impl> _impl;

};

END_NAMESPACE_GMGRAPHICS;

#endif
