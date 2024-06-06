
#ifndef GRAMODS_GRAPHICS_OFFSCREENRENDERTARGETS
#define GRAMODS_GRAPHICS_OFFSCREENRENDERTARGETS

#include <gmGraphics/config.hh>
#include <gmCore/io_size.hh>

#include <GL/glew.h>
#include <GL/gl.h>

#include <memory>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   Encapsulation of OpenGL offscreen render target textures. There is
   support for multiple textures, but there will always be only one
   depth buffer.
*/
class OffscreenRenderTargets {

public:

  OffscreenRenderTargets();
  virtual ~OffscreenRenderTargets();

  /**
     Set to true if even power of two should be used for texture size,
     instead of the size provided to bind. The viewport size will
     always be set to the specified size. Default it false.
  */
  void setUsePowersOfTwo(bool on);

  /**
     Returns true when even power of two is used for texture size,
     instead of the size provided to bind. The viewport size will
     always be set to the specified size.
  */
  bool getUsePowersOfTwo();

  /**
     Turn on or off linear interpolation in the texture(s). Default is
     off.
  */
  void setLinearInterpolation(bool on);

  /**
     Set the pixel format to use for the color buffer(s). Default is
     GL_RGBA8.
  */
  void setPixelFormat(GLenum format);

  /**
     Return the pixel format currently used for the color buffer(s).
  */
  GLenum getPixelFormat();

  /**
     Provides the size of the specified texture from the last bind
     call. This will be the same as the size provided to the last bind
     call, unless use powers of two is set to true. This will then be
     the next power of two.
  */
  void getTextureSize(size_t &width, size_t &height, size_t idx = 0);

  /**
     Called with GL context to initialize the object. Returns true if
     the initialization was successful, false otherwise.

     @param[in] count The number of offscreen render target textures
     to generate space for. Default is 1 (one)
  */
  bool init(size_t count = 1);

  /**
     saves away current viewport and render target data.
  */
  void push();

  /**
     restores previously saved viewport and render target data. Do not
     interlace calls to push and pop.
  */
  void pop();

  /**
     Sets the texture resolution and sets this texture as render
     target.
  */
  void bind(gmCore::size2 res, size_t idx = 0);

  /**
     Sets the texture resolution and sets this texture as render
     target.
  */
  void bind(size_t width = 0, size_t height = 0, size_t idx = 0);

  /**
     Returns the texture id of the specified offline render buffer.
  */
  GLuint getTexId(size_t idx = 0);

private:

  struct Impl;
  std::unique_ptr<Impl> _impl;

};

END_NAMESPACE_GMGRAPHICS;

#endif
