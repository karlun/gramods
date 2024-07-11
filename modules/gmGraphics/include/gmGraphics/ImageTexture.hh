
#ifndef GRAMODS_GRAPHICS_IMAGETEXTURE
#define GRAMODS_GRAPHICS_IMAGETEXTURE

#include <gmGraphics/config.hh>

#ifdef gramods_ENABLE_FreeImage

// Required before gmCore/OFactory.hh for some compilers
#include <gmCore/io_size.hh>

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
  virtual ~ImageTexture();

  void initialize() override;

  /**
     Called by Updateable::updateAll to increment the frame counter.
  */
  void update(clock::time_point t, size_t) override;

  /**
     Updates the texture and returns the ID of the associated GL
     texture object. Must be called with GL context. Observe also that
     this method may perform off-screen rendering.

     @param frame_number The current frame being rendered. This number
     should increment by one for each frame, however it may wrap
     around and start over at zero.

     @param eye Eye for which the texture is requested. An
     implementation may use different textures for different eyes or
     may reuse the same texture for all eyes.

     @returns OpenGL texture ID of the updated texture
  */
  GLuint updateTexture(size_t frame_number, Eye eye) override;

  /**
     Set the file to read image data from. If a sequence of image
     files should be read, set this as a fprint formatted template and
     use setRange to specify the range. For example if file is set to
     "image_%05d.png" and range is set to 1-199, then images from
     "image_00000.png" to "image_00199.png" will be read.

     \gmXmlTag{gmGraphics,ImageTexture,file}
  */
  void setFile(std::filesystem::path file);

  /**
     Returns the currently set file path or pattern.
  */
  std::filesystem::path getFile();

  /**
     Sets the range (inclusive) of frames to read. This assumes that
     the file name specified with setFile is a printf formatted
     template.

     \gmXmlTag{gmGraphics,ImageTexture,range}
  */
  void setRange(gmCore::size2 range);

  /**
     Activates auto range, with which the range of images is guessed
     based on the available files with the specified name pattern.
  */
  void setAutoRange(bool on);

  /**
     Activates or deactivates looping the animation. Default is false.

     \gmXmlTag{gmGraphics,ImageTexture,loop}
  */
  void setLoop(bool on);

  /**
     Activates or deactivates logging progress information. Default is false.

     \gmXmlTag{gmGraphics,ImageTexture,logProgress}
  */
  void setLogProgress(bool on);

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
