
#ifndef GRAMODS_GRAPHICS_GLUTILS
#define GRAMODS_GRAPHICS_GLUTILS

#include <gmGraphics/config.hh>

#include <gmCore/Console.hh>

#include <GL/glew.h>
#include <GL/gl.h>

#include <string>

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

  static std::string toString(GLenum v);

  /**
     Check if the OpenGL state is good and in that case return
     true. If there has been an error then false is returned. If msg
     is set, then the this string is logged as error together with the
     error flag returned by glGetError(). This call will empty the
     error queue.
  */
  static bool isStateGood(std::optional<std::string> msg = std::nullopt) {
    bool good = true;
    while(true) {
      auto err = glGetError();
      if (err == GL_NO_ERROR) return good;

      good = false;
      if (msg) GM_ERR("OpenGL", *msg << toString(err));
    }
  }
};

END_NAMESPACE_GMGRAPHICS;

#endif
