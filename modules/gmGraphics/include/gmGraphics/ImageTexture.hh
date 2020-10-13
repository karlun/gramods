
#ifndef GRAMODS_GRAPHICS_IMAGETEXTURE
#define GRAMODS_GRAPHICS_IMAGETEXTURE

#include <gmGraphics/config.hh>

#ifdef gramods_ENABLE_FreeImage

#include <gmCore/size.hh>
#include <gmCore/OFactory.hh>
#include <gmCore/Updateable.hh>
#include <gmGraphics/Texture.hh>
#include <memory>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   The ImageTexture reads image data from a specified file to populate
   its texture data.
*/
class ImageTexture
  : public gmGraphics::Texture,
    public gmCore::Updateable {

public:

  ImageTexture();

  void initialize();

  /**
     Updates the live texture with new data. Must be called with GL
     context.
  */
  void update();

  /**
     Called by Updateable::updateAll to increment the frame counter.
  */
  void update(clock::time_point t);

  /**
     Returns the ID of the associated GL texture object.
  */
  GLuint getGLTextureID();

  /**
     Set the file to read image data from. If a sequence of image
     files should be read, set this as a fprint formatted template and
     use setRange to specify the range. For example if file is set to
     "image_%05d.png" and range is set to 1-199, then images from
     "image_00000.png" to "image_00199.png" will be read.

     \gmXmlTag{gmGraphics,ImageTexture,file}
  */
  void setFile(std::string file);

  /**
     Sets the range (inclusive) of frames to read. This assumes that
     the file name specified with setFile is a printf formatted
     template.

     \gmXmlTag{gmGraphics,ImageTexture,range}
  */
  void setRange(gmCore::size2 range);

  /**
     Activates or deactivates looping the animation. Default is false.

     \gmXmlTag{gmGraphics,ImageTexture,loop}
  */
  void setLoop(bool on);

  /**
     Activates or deactivates automatic exit when the specified range
     has been covered. Default is false. Loop has to be false for this
     to work.

     \gmXmlTag{gmGraphics,ImageTexture,exit}
  */
  void setExit(bool on);

  GM_OFI_DECLARE;

private:

  struct Impl;
  std::unique_ptr<Impl> _impl;
};

END_NAMESPACE_GMGRAPHICS;

#endif
#endif
