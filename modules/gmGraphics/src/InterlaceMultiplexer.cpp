
#include <gmGraphics/InterlaceMultiplexer.hh>

#include <gmGraphics/GLUtils.hh>
#include <gmCore/Console.hh>

#include <GL/glew.h>
#include <GL/gl.h>

#define PATTERN_HORIZONTAL_LINES 0
#define PATTERN_VERTICAL_LINES 1
#define PATTERN_CHECKERBOARD 2

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE(InterlaceMultiplexer);
GM_OFI_PARAM2(InterlaceMultiplexer, pattern, int, setPattern);

struct InterlaceMultiplexer::Impl {

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
  GLint target_framebuffer;

  int pattern = 0;

  void setup();
  void teardown();

  void prepare();
  void setupRendering(size_t eye);
  void finalize();
};

InterlaceMultiplexer::InterlaceMultiplexer()
  : _impl(std::make_unique<InterlaceMultiplexer::Impl>()) {}

InterlaceMultiplexer::Impl::~Impl() {
  teardown();
}

void InterlaceMultiplexer::prepare() {
  _impl->prepare();
}

void InterlaceMultiplexer::setupRendering(size_t eye) {
  _impl->setupRendering(eye);
}

void InterlaceMultiplexer::finalize() {
  _impl->finalize();
}

void InterlaceMultiplexer::setPattern(int p) {
  if (p < 0 || 2 < p)
    throw gmCore::InvalidArgument("invalid interlace pattern");
  _impl->pattern = p;
}

void InterlaceMultiplexer::Impl::setup() {
  is_setup = true;
  is_functional = false;

  GM_DBG2("InterlaceMultiplexer", "Creating buffers and textures");
  glGenFramebuffers((GLsizei)2, fb_id);
  glGenTextures((GLsizei)2, tex_id);
  glGenRenderbuffers(1, &rb_depth_id);

  for (size_t eye_idx = 0; eye_idx < 2; ++eye_idx) {
    glBindFramebuffer(GL_FRAMEBUFFER, fb_id[eye_idx]);
    glBindTexture(GL_TEXTURE_2D, tex_id[eye_idx]);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 32, 32, 0,GL_RGB, GL_UNSIGNED_BYTE, 0);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_id[eye_idx], 0);

    glBindRenderbuffer(GL_RENDERBUFFER, rb_depth_id);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32F, 32, 32);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rb_depth_id);

    if (!GLUtils::check_framebuffer())
      return;
  }

  static const char * vertex_shader_code = R"lang=glsl(
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
)lang=glsl";

  GM_DBG2("InterlaceMultiplexer", "Creating vertex shader");
  vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertex_shader_id, 1, &vertex_shader_code, nullptr);
  glCompileShader(vertex_shader_id);

  static const char * fragment_shader_code = R"lang=glsl(
#version 330 core

uniform sampler2D texL;
uniform sampler2D texR;

uniform int pattern;

uniform int width;
uniform int height;

in vec2 v_uv;
out vec4 fragColor;

void main() {

  if (pattern == 0) {

    int idx = int(gl_FragCoord.y);

    if (mod(idx, 2) == 0) fragColor = texture(texL, v_uv);
    else fragColor = texture(texR, v_uv);

  } else if (pattern == 1) {

    int idx = int(gl_FragCoord.x);
    if (mod(idx, 2) == 0) fragColor = texture(texL, v_uv);
    else fragColor = texture(texR, v_uv);

  } else if (pattern == 2) {

    int idxH = int(gl_FragCoord.x);
    int idxV = int(gl_FragCoord.y);
    if (mod(idxH + idxV, 2) == 0) fragColor = texture(texL, v_uv);
    else fragColor = texture(texR, v_uv);

  }

  return;
}
)lang=glsl";

  GM_DBG2("InterlaceMultiplexer", "Creating fragment shader");
  fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment_shader_id, 1, &fragment_shader_code, nullptr);
  glCompileShader(fragment_shader_id);

  GM_DBG2("InterlaceMultiplexer", "Creating and linking program");
  program_id = glCreateProgram();
  glAttachShader(program_id, vertex_shader_id);
  glAttachShader(program_id, fragment_shader_id);
  glLinkProgram(program_id);
  glBindAttribLocation(program_id, 0, "in_Position");

  if (!GLUtils::check_shader_program(program_id))
    return;

  GM_DBG2("InterlaceMultiplexer", "Creating vertex array");
  glGenVertexArrays(1, &vao_id);
  glBindVertexArray(vao_id);

  GM_DBG2("InterlaceMultiplexer", "Creating and setting up array buffer");
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

void InterlaceMultiplexer::Impl::teardown() {
  is_functional = false;

  if (fb_id[0]) glDeleteFramebuffers((GLsizei)2, fb_id);
  if (tex_id[0]) glDeleteTextures((GLsizei)2, tex_id);
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

void InterlaceMultiplexer::Impl::prepare() {

  glGetIntegerv(GL_FRAMEBUFFER_BINDING, &target_framebuffer);
  glGetIntegerv(GL_VIEWPORT, viewport);

  port_width = viewport[2];
  port_height = viewport[3];

  tex_width = GLUtils::nextPowerOfTwo(port_width);
  tex_height = GLUtils::nextPowerOfTwo(port_height);

  if (!is_setup)
    setup();
  if (!is_functional)
    return;
}

void InterlaceMultiplexer::Impl::setupRendering(size_t eye) {

  if (eye >= 2) throw gmCore::InvalidArgument("cannot render eye index higher than 1");

  if (!is_functional)
    return;

  GM_DBG2("InterlaceMultiplexer", "setting up for "
          << (eye == 0 ? "left eye" : "right eye"));

  glBindFramebuffer(GL_FRAMEBUFFER, fb_id[(size_t)eye]);

  GM_DBG2("InterlaceMultiplexer", "Allocating frame buffer texture " << tex_width << "x" << tex_height << " for port " << port_width << "x" << port_height);
  glBindTexture(GL_TEXTURE_2D, tex_id[(size_t)eye]);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex_width, tex_height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
  glBindTexture(GL_TEXTURE_2D, 0);

  glBindRenderbuffer(GL_RENDERBUFFER, rb_depth_id);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32F, tex_width, tex_height);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);

  glViewport(0, 0, port_width, port_height);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void InterlaceMultiplexer::Impl::finalize() {
  GM_DBG2("InterlaceMultiplexer", "finalizing");

  glBindFramebuffer(GL_FRAMEBUFFER, target_framebuffer);
  glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, tex_id[0]);

  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, tex_id[1]);

  glUseProgram(program_id);
  glUniform1i(glGetUniformLocation(program_id, "texL"), 0);
  glUniform1i(glGetUniformLocation(program_id, "texR"), 1);
  glUniform1f(glGetUniformLocation(program_id, "dx"), port_width/(float)tex_width);
  glUniform1f(glGetUniformLocation(program_id, "dy"), port_height/(float)tex_height);
  glUniform1i(glGetUniformLocation(program_id, "pattern"), pattern);

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
