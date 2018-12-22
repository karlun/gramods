
#include <gmGraphics/GLUtils.hh>

#include <gmCore/Console.hh>

#include <GL/glew.h>
#include <GL/gl.h>

BEGIN_NAMESPACE_GMGRAPHICS;

bool GLUtils::check_shader_program(GLuint program_id) {

  GLint msg_data_len;
  glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &msg_data_len);

  if (!msg_data_len)
    msg_data_len = 64;

  std::vector<GLchar> msg_data(msg_data_len, '\0');
  
  GLint msg_len;
  glGetProgramInfoLog(program_id, msg_data_len, &msg_len, &msg_data[0]);

  GLint status;
  glValidateProgram(program_id);
  glGetProgramiv(program_id, GL_VALIDATE_STATUS, &status);

  if (status) {
    if (msg_len)
      GM_INF("OpenGL", &msg_data[0]);
    return true;
  }

  if (!msg_len)
    GM_ERR("OpenGL", "shader program not valid");
  else
    GM_ERR("OpenGL", &msg_data[0]);

  return false;
}

bool GLUtils::check_framebuffer() {
	GLenum status;
	status = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
	switch(status) {
	case GL_FRAMEBUFFER_COMPLETE:
    GM_INF("SimpleAnaglyphsMultiplexer", "Frame buffer complete");
		return true;

	case GL_FRAMEBUFFER_UNSUPPORTED:
    GM_ERR("SimpleAnaglyphsMultiplexer", "Frame buffer unsupported");
		break;

	default:
    GM_ERR("SimpleAnaglyphsMultiplexer", "Frame buffer incomplete");
	}
  return false;
}

int GLUtils::nextPowerOfTwo(int v) {
  v--;
  v |= v >> 1;
  v |= v >> 2;
  v |= v >> 4;
  v |= v >> 8;
  v |= v >> 16;
  v++;
  return v;
}


END_NAMESPACE_GMGRAPHICS;
