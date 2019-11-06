
#ifndef GRAMODS_GRAPHICS_RASTERPROCESSOR
#define GRAMODS_GRAPHICS_RASTERPROCESSOR

#include <gmGraphics/config.hh>

#include <string>
#include <memory>

#include <GL/glew.h>
#include <GL/gl.h>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   Encapsulated OpenGL code for executing a specified fragment shader
   on every raster point in the current render target.
*/
class RasterProcessor {

public:

  RasterProcessor();
  virtual ~RasterProcessor();

  /**
     Set the vertex shader code to run in the raster processor. This
     must be called before init. The default code will set
     gl_Position to (x y 0 1) where x and y are in the range -1 to 1.
  */
  void setVertexCode(std::string code);

  /**
     Set the fragment shader code to run in the raster processor. This
     must be called before init. This must be set.
  */
  void setFragmentCode(std::string code);

  /**
     Initialize the raster processor.
  */
  bool init();

  /**
     Execute the raster processor on the current render target.
  */
  void run();

  /**
     Returns the program id. The raster processor must be
     initialized. Otherwise this method will return zero.
   */
  GLuint getProgramId();

private:

  struct Impl;
  std::unique_ptr<Impl> _impl;

};

END_NAMESPACE_GMGRAPHICS;

#endif
