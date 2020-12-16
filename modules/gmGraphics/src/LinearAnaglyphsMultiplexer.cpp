
#include <gmGraphics/LinearAnaglyphsMultiplexer.hh>

#include <gmGraphics/GLUtils.hh>
#include <gmCore/Console.hh>

#include <GL/glew.h>
#include <GL/gl.h>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE(LinearAnaglyphsMultiplexer);
GM_OFI_PARAM2(LinearAnaglyphsMultiplexer, leftColor, gmCore::float3, setLeftColor);
GM_OFI_PARAM2(LinearAnaglyphsMultiplexer, rightColor, gmCore::float3, setRightColor);
GM_OFI_PARAM2(LinearAnaglyphsMultiplexer, saturation, float, setSaturation);
GM_OFI_PARAM2(LinearAnaglyphsMultiplexer, leftSaturation, float, setLeftSaturation);
GM_OFI_PARAM2(LinearAnaglyphsMultiplexer, rightSaturation, float, setRightSaturation);

struct LinearAnaglyphsMultiplexer::Impl {

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

  gmCore::float3 left_color = { 1.f, 0.f, 0.f };
  gmCore::float3 right_color = { 0.f, 1.f, 1.f };
  float left_saturation = 0.8f;
  float right_saturation = 0.8f;

  void setup();
  void teardown();

  void prepare();
  void setupRendering(size_t eye);
  void finalize();
};

LinearAnaglyphsMultiplexer::LinearAnaglyphsMultiplexer()
  : _impl(std::make_unique<LinearAnaglyphsMultiplexer::Impl>()) {}

LinearAnaglyphsMultiplexer::Impl::~Impl() {
  teardown();
}

void LinearAnaglyphsMultiplexer::prepare() {
  _impl->prepare();
}

void LinearAnaglyphsMultiplexer::setupRendering(size_t eye) {
  _impl->setupRendering(eye);
}

void LinearAnaglyphsMultiplexer::finalize() {
  _impl->finalize();
}

void LinearAnaglyphsMultiplexer::setLeftColor(gmCore::float3 c) {
  _impl->left_color = c;
}

void LinearAnaglyphsMultiplexer::setRightColor(gmCore::float3 c) {
  _impl->right_color = c;
}

void LinearAnaglyphsMultiplexer::setSaturation(float s) {
  _impl->left_saturation = s;
  _impl->right_saturation = s;
}

void LinearAnaglyphsMultiplexer::setLeftSaturation(float s) {
  _impl->left_saturation = s;
}

void LinearAnaglyphsMultiplexer::setRightSaturation(float s) {
  _impl->right_saturation = s;
}

void LinearAnaglyphsMultiplexer::Impl::setup() {
  is_setup = true;
  is_functional = false;

  GM_DBG2("LinearAnaglyphsMultiplexer", "Creating buffers and textures");
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

  GM_DBG2("LinearAnaglyphsMultiplexer", "Creating vertex shader");
  vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertex_shader_id, 1, &vertex_shader_code, nullptr);
  glCompileShader(vertex_shader_id);

  static const char * fragment_shader_code = R"lang=glsl(
#version 330 core

uniform sampler2D texL;
uniform sampler2D texR;

uniform vec3 left_color;
uniform vec3 right_color;
uniform float left_saturation;
uniform float right_saturation;

in vec2 v_uv;
out vec4 fragColor;

// Digital ITU BT.601
vec3 togray = vec3(0.299, 0.587, 0.114);

void main() {

  vec3 colL = texture(texL, v_uv).rgb;
  vec3 colR = texture(texR, v_uv).rgb;

  float grL = dot(colL, togray);
  float grR = dot(colR, togray);

  vec3 L = mix(vec3(grL, grL, grL), colL, left_saturation);
  vec3 R = mix(vec3(grR, grR, grR), colR, right_saturation);

  fragColor = vec4(L * left_color + R * right_color, 1);
}
)lang=glsl";

  GM_DBG2("LinearAnaglyphsMultiplexer", "Creating fragment shader");
  fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment_shader_id, 1, &fragment_shader_code, nullptr);
  glCompileShader(fragment_shader_id);

  GM_DBG2("LinearAnaglyphsMultiplexer", "Creating and linking program");
  program_id = glCreateProgram();
  glAttachShader(program_id, vertex_shader_id);
  glAttachShader(program_id, fragment_shader_id);
  glLinkProgram(program_id);
  glBindAttribLocation(program_id, 0, "in_Position");

  if (!GLUtils::check_shader_program(program_id))
    return;

  GM_DBG2("LinearAnaglyphsMultiplexer", "Creating vertex array");
  glGenVertexArrays(1, &vao_id);
  glBindVertexArray(vao_id);

  GM_DBG2("LinearAnaglyphsMultiplexer", "Creating and setting up array buffer");
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

void LinearAnaglyphsMultiplexer::Impl::teardown() {
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

void LinearAnaglyphsMultiplexer::Impl::prepare() {

  glGetIntegerv(GL_FRAMEBUFFER_BINDING, &target_framebuffer);
  glGetIntegerv(GL_VIEWPORT, viewport);

  port_width = viewport[2];
  tex_width = GLUtils::nextPowerOfTwo(port_width);
  port_height = viewport[3];
  tex_height = GLUtils::nextPowerOfTwo(port_height);

  if (!is_setup)
    setup();
  if (!is_functional)
    return;
}

void LinearAnaglyphsMultiplexer::Impl::setupRendering(size_t eye) {

  if (eye >= 2) throw gmCore::InvalidArgument("cannot render eye index higher than 1");

  if (!is_functional)
    return;

  GM_DBG2("LinearAnaglyphsMultiplexer", "setting up for "
          << (eye == 0 ? "left eye" : "right eye"));

  glBindFramebuffer(GL_FRAMEBUFFER, fb_id[(size_t)eye]);

  GM_DBG2("LinearAnaglyphsMultiplexer", "Allocating frame buffer texture " << tex_width << "x" << tex_height << " for port " << port_width << "x" << port_height);
  glBindTexture(GL_TEXTURE_2D, tex_id[(size_t)eye]);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex_width, tex_height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
  glBindTexture(GL_TEXTURE_2D, 0);

  glBindRenderbuffer(GL_RENDERBUFFER, rb_depth_id);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32F, tex_width, tex_height);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);

  glViewport(0, 0, port_width, port_height);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void LinearAnaglyphsMultiplexer::Impl::finalize() {
  GM_DBG2("LinearAnaglyphsMultiplexer", "finalizing");

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
  glUniform3fv(glGetUniformLocation(program_id, "left_color"), 1, left_color.data());
  glUniform3fv(glGetUniformLocation(program_id, "right_color"), 1, right_color.data());
  glUniform1f(glGetUniformLocation(program_id, "left_saturation"), left_saturation);
  glUniform1f(glGetUniformLocation(program_id, "right_saturation"), right_saturation);

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
