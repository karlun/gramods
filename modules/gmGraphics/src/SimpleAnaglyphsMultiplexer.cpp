
#include <gmGraphics/SimpleAnaglyphsMultiplexer.hh>

#include <GL/glew.h>
#include <GL/gl.h>

#include <gmGraphics/maths.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE(SimpleAnaglyphsMultiplexer);

struct SimpleAnaglyphsMultiplexer::Impl {

  ~Impl();

  bool is_setup = false;
  bool is_functional = false;

  GLuint fb_id[2] = { 0, 0 };
  GLuint tex_id[2] = { 0, 0 };
  GLuint rb_depth_id = 0;

  GLuint vertex_shader_id = 0;
  GLuint fragment_shader_id = 0;
  GLuint program_id = 0;
  GLuint vao_id = 0;
  GLuint vbo_id = 0;

  GLuint tex_width, tex_height;
  GLuint port_width, port_height;
  GLint viewport[4] = { 0, 0, 0, 0 };

  void setup();
  void teardown();
  bool verify_framebuffer();

  void prepare();
  void setupRendering(Eye eye);
  void finalize();
};

SimpleAnaglyphsMultiplexer::SimpleAnaglyphsMultiplexer()
  : _impl(std::make_unique<SimpleAnaglyphsMultiplexer::Impl>()) {}

SimpleAnaglyphsMultiplexer::Impl::~Impl() {
  teardown();
}

void SimpleAnaglyphsMultiplexer::prepare() {
  _impl->prepare();
}

void SimpleAnaglyphsMultiplexer::setupRendering(Eye eye) {
  _impl->setupRendering(eye);
}

void SimpleAnaglyphsMultiplexer::finalize() {
  _impl->finalize();
}

bool SimpleAnaglyphsMultiplexer::Impl::verify_framebuffer() {
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

void SimpleAnaglyphsMultiplexer::Impl::setup() {
  is_setup = true;
  is_functional = false;

  GM_VINF("SimpleAnaglyphsMultiplexer", "Creating buffers and textures");
  glGenFramebuffers((GLsizei)Eye::COUNT, fb_id);
  glGenTextures((GLsizei)Eye::COUNT, tex_id);
  glGenRenderbuffers(1, &rb_depth_id);

  for (size_t eye_idx = (size_t)Eye::LEFT; eye_idx <= (size_t)Eye::RIGHT; ++eye_idx) {
    glBindFramebuffer(GL_FRAMEBUFFER, fb_id[eye_idx]);
    glBindTexture(GL_TEXTURE_2D, tex_id[eye_idx]);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 32, 32, 0,GL_RGB, GL_UNSIGNED_BYTE, 0);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_id[eye_idx], 0);

#if 0
    glBindRenderbuffer(GL_RENDERBUFFER, rb_depth_id);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32F, 32, 32);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rb_depth_id);
#endif

    if (!verify_framebuffer())
      return;
  }

  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  static const char * vertex_shader_code = R"(
#version 330 core

uniform float dx;
uniform float dy;

in vec2 a_vertex;
out vec2 v_uv;

void main() {
  v_uv = vec2(dx * (a_vertex.x * 0.5 + 0.5),
              dy * (a_vertex.y * 0.5 + 0.5));
  gl_Position = vec4(a_vertex, 0.0, 1.0);
}
)";

  GM_VINF("SimpleAnaglyphsMultiplexer", "Creating vertex shader");
  vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertex_shader_id, 1, &vertex_shader_code, nullptr);
  glCompileShader(vertex_shader_id);

  static const char * fragment_shader_code = R"(
#version 330 core

uniform sampler2D texL;
uniform sampler2D texR;

in vec2 v_uv;
out vec4 fragColor;

void main() {
  fragColor = 0.5 * vec4(texture(texL, v_uv).rgb, 1) + 0.5 * vec4(texture(texR, v_uv).rgb, 1);
}
)";

  GM_VINF("SimpleAnaglyphsMultiplexer", "Creating fragment shader");
  fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment_shader_id, 1, &fragment_shader_code, nullptr);
  glCompileShader(fragment_shader_id);

  GM_VINF("SimpleAnaglyphsMultiplexer", "Creating and linking program");
  program_id = glCreateProgram();
  glAttachShader(program_id, vertex_shader_id);
  glAttachShader(program_id, fragment_shader_id);
  glLinkProgram(program_id);
  glBindAttribLocation(program_id, 0, "in_Position");
    
  GM_VINF("SimpleAnaglyphsMultiplexer", "Creating vertex array");
  glGenVertexArrays(1, &vao_id);
  glBindVertexArray(vao_id);

  GM_VINF("SimpleAnaglyphsMultiplexer", "Creating and setting up array buffer");
  glGenBuffers(1, &vbo_id);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
  const GLfloat vertices[4][2] = {
    { +1.0, -1.0  },
    { +1.0, +1.0  },
    { -1.0, -1.0  },
    { -1.0, +1.0  } };
  glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(GLfloat), vertices, GL_STATIC_DRAW);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  is_functional = true;
}

void SimpleAnaglyphsMultiplexer::Impl::teardown() {
  is_functional = false;

  if (fb_id[0]) glDeleteFramebuffers((GLsizei)Eye::COUNT, fb_id);
  if (tex_id[0]) glDeleteTextures((GLsizei)Eye::COUNT, tex_id);
  if (rb_depth_id) glDeleteRenderbuffers(1, &rb_depth_id);

  fb_id[0] = 0;
  tex_id[0] = 0;
  rb_depth_id = 0;

  if (program_id) glDeleteProgram(program_id);
  if (vertex_shader_id) glDeleteShader(vertex_shader_id);
  if (fragment_shader_id) glDeleteShader(fragment_shader_id);
  if (vao_id) glDeleteVertexArrays(1, &vao_id);
  if (vbo_id) glDeleteBuffers(1, &vbo_id);

  vertex_shader_id = 0;
  fragment_shader_id = 0;
  program_id = 0;
  vao_id = 0;
  vbo_id = 0;

  is_setup = false;
}

void SimpleAnaglyphsMultiplexer::Impl::prepare() {
  if (!is_setup)
    setup();
  if (!is_functional)
    return;

  glGetIntegerv(GL_VIEWPORT, viewport);
  port_width = viewport[2];
  tex_width = gmGraphics::nextPowerOfTwo(port_width);
  port_height = viewport[3];
  tex_height = gmGraphics::nextPowerOfTwo(port_height);
}

void SimpleAnaglyphsMultiplexer::Impl::setupRendering(Eye eye) {
  if (!is_functional)
    return;

  GM_VINF("SimpleAnaglyphsMultiplexer", "setting up for "
          << (eye == Eye::LEFT ? "left eye" : "right eye"));

  glBindFramebuffer(GL_FRAMEBUFFER, fb_id[(size_t)eye]);

  GM_VINF("SimpleAnaglyphsMultiplexer", "Allocating frame buffer texture " << tex_width << "x" << tex_height << " for port " << port_width << "x" << port_height);
  glBindTexture(GL_TEXTURE_2D, tex_id[(size_t)eye]);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tex_width, tex_height, 0,GL_RGB, GL_UNSIGNED_BYTE, 0);
  glBindTexture(GL_TEXTURE_2D, 0);

#if 0
  glBindRenderbuffer(GL_RENDERBUFFER, rb_depth_id);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32F, tex_width, tex_height);
#endif

  glViewport(0, 0, port_width, port_height);
}

void SimpleAnaglyphsMultiplexer::Impl::finalize() {
  GM_VINF("SimpleAnaglyphsMultiplexer", "finalizing");

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, tex_id[(size_t)Eye::LEFT]);

  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, tex_id[(size_t)Eye::RIGHT]);

  glUseProgram(program_id);
  glUniform1i(glGetUniformLocation(program_id, "texL"), 0);
  glUniform1i(glGetUniformLocation(program_id, "texR"), 1);
  glUniform1f(glGetUniformLocation(program_id, "dx"), port_width/(float)tex_width);
  glUniform1f(glGetUniformLocation(program_id, "dy"), port_height/(float)tex_height);

  glBindVertexArray(vao_id);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  glBindVertexArray(0);

  glUseProgram(0);
  glDisableVertexAttribArray(0);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture( GL_TEXTURE_2D, 0);

  glActiveTexture(GL_TEXTURE1);
  glBindTexture( GL_TEXTURE_2D, 0);
}

END_NAMESPACE_GMGRAPHICS;
