
#ifndef GRAMODS_GRAPHICS_IMAGETEXTURE
#define GRAMODS_GRAPHICS_IMAGETEXTURE

#include <gmGraphics/config.hh>

#ifdef gramods_ENABLE_FreeImage

#include <gmCore/OFactory.hh>
#include <gmGraphics/Texture.hh>
#include <memory>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   The ImageTexture reads image data from a specified file to populate
   its texture data.
*/
class ImageTexture
  : public gmGraphics::Texture {

public:

  ImageTexture();

  void initialize();

  /**
     Updates the live texture with new data. Must be called with GL
     context.
  */
  void update();

  /**
     Returns the ID of the associated GL texture object.
  */
  GLuint getGLTextureID();

  /**
     Set the file to read image data from.
   */
  void setFile(std::string file) { this->file = file; }

  GM_OFI_DECLARE;

private:

  class _This;
  std::unique_ptr<_This> _this;

  std::string file;

};

END_NAMESPACE_GMGRAPHICS;

#endif
#endif
