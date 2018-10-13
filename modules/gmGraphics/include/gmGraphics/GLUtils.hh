
#ifndef GRAMODS_GRAPHICS_GLUTILS
#define GRAMODS_GRAPHICS_GLUTILS

#include <gmGraphics/config.hh>

#include <GL/glew.h>
#include <GL/gl.h>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   Utility functions for handling GL.
*/
struct GLUtils {

  /**
     Checks the status of the specified shader program and returns true
     if its last validation status was successful.
  */
  static bool check_shader_program(GLuint program_id);

  /**
     Checks the status of the current frame buffer and returns true if
     it is complete.
  */
  static bool check_framebuffer();

  /**
     Quickly calculates the next power of two.
  */
  static int nextPowerOfTwo(int v);

};

END_NAMESPACE_GMGRAPHICS;

#endif
