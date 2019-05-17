
#ifndef GRAMODS_GRAPHICS_OFFLINEBUFFER
#define GRAMODS_GRAPHICS_OFFLINEBUFFER

#include <gmGraphics/config.hh>

#include <GL/glew.h>
#include <GL/gl.h>

#include <memory>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   Encapsulation of OpenGL offline buffer.
*/
class OfflineRenderTarget {

public:

  OfflineRenderTarget();

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
  void bind(size_t width, size_t height);

  /**
     Unbinds the
  */
  void unbind();

  /**
     Returns the texture id of the offline render buffer.
  */
  GLuint getTexId();

private:

  class Impl;
  std::unique_ptr<Impl> _impl;

};

END_NAMESPACE_GMGRAPHICS;

#endif
