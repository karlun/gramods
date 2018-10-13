
#include <gmGraphics/EquirectangularView.hh>

#include <gmGraphics/GLUtils.hh>

#include <GL/glew.h>
#include <GL/gl.h>

#include <vector>
#include <Eigen/Eigen>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE_SUB(EquirectangularView, View);
GM_OFI_PARAM(EquirectangularView, cubeMapResolution, int, EquirectangularView::setCubeMapResolution);

struct EquirectangularView::Impl {

  ~Impl() {
    teardown();
  }

  static const size_t SIDE_COUNT = 6;

  Eigen::Quaternionf side_orientation[SIDE_COUNT] = {
    Eigen::Quaternionf(Eigen::Quaternionf::AngleAxisType( gramods_PI_2, Eigen::Vector3f::UnitY())),
    Eigen::Quaternionf(Eigen::Quaternionf::AngleAxisType(-gramods_PI_2, Eigen::Vector3f::UnitY())),
    Eigen::Quaternionf(Eigen::Quaternionf::AngleAxisType(-gramods_PI_2, Eigen::Vector3f::UnitX())),
    Eigen::Quaternionf(Eigen::Quaternionf::AngleAxisType( gramods_PI_2, Eigen::Vector3f::UnitX())),
    Eigen::Quaternionf(Eigen::Quaternionf::AngleAxisType( gramods_PI  , Eigen::Vector3f::UnitY())),
    Eigen::Quaternionf::Identity()
  };

  bool is_setup = false;
  bool is_functional = false;

  int resolution = 2048;

  GLuint fb_id[SIDE_COUNT] = { 0 };
  GLuint tex_id[SIDE_COUNT] = { 0 };
  GLuint rb_depth_id = 0;

  GLuint vertex_shader_id = 0;
  GLuint fragment_shader_id = 0;
  GLuint program_id = 0;
  GLuint vao_id = 0;
  GLuint vbo_id = 0;

  GLint viewport[4] = { 0, 0, 0, 0 };

  void setup();
  void teardown();

  void renderFullPipeline(ViewSettings settings);
  void renderSide(ViewSettings settings, size_t side);

  Eigen::Vector3f position = Eigen::Vector3f::Zero();
};

EquirectangularView::EquirectangularView()
  : _impl(std::make_unique<Impl>()) {}

void EquirectangularView::renderFullPipeline(ViewSettings settings) {
  populateViewSettings(settings);
  _impl->renderFullPipeline(settings);
}

void EquirectangularView::Impl::setup() {
  is_setup = true;
  is_functional = false;

  if (resolution != GLUtils::nextPowerOfTwo(resolution)) {
    GM_WRN("EquirectangularView", "Cube map resolution (" << resolution << ") is not an even power of two");
  }

  GM_VINF("EquirectangularView", "Creating buffers and textures");
  glGenFramebuffers(SIDE_COUNT, fb_id);
  glGenTextures(SIDE_COUNT, tex_id);
  glGenRenderbuffers(1, &rb_depth_id);

  for (size_t idx = 0; idx < SIDE_COUNT; ++idx) {
    glBindFramebuffer(GL_FRAMEBUFFER, fb_id[idx]);
    glBindTexture(GL_TEXTURE_2D, tex_id[idx]);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, resolution, resolution, 0,GL_RGB, GL_UNSIGNED_BYTE, 0);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_id[idx], 0);

    glBindRenderbuffer(GL_RENDERBUFFER, rb_depth_id);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32F, resolution, resolution);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rb_depth_id);

    if (!GLUtils::check_framebuffer())
      return;
  }

  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  static const char * vertex_shader_code = R"(
#version 330 core

in vec2 a_vertex;
out vec2 pos;

void main() {
  pos = a_vertex;
  gl_Position = vec4(a_vertex, 0.0, 1.0);
}
)";

  GM_VINF("EquirectangularView", "Creating vertex shader");
  vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertex_shader_id, 1, &vertex_shader_code, nullptr);
  glCompileShader(vertex_shader_id);

  static const char * fragment_shader_code = R"(
#version 330 core

uniform sampler2D texLeft;
uniform sampler2D texRight;
uniform sampler2D texBottom;
uniform sampler2D texTop;
uniform sampler2D texBack;
uniform sampler2D texFront;

in vec2 pos;
out vec4 fragColor;

void colorFromTex(float x, float y, float z, sampler2D tex) {
  fragColor = vec4(texture(tex, 0.5 * vec2(x/z, y/z) + 0.5).rgb, 1);
}

void main() {
  
  float ay = pos.y * 1.57079632679489661923;
  float ax = pos.x * 3.14159265358979323846;

  vec3 pix = vec3(cos(ay) * sin(ax), sin(ay), -cos(ay) * cos(ax));

  if (pix.x < -abs(pix.y) && pix.x < -abs(pix.z)) {
    colorFromTex(-pix.z,  pix.y, -pix.x, texLeft);
    return;
  }

  if (pix.x >  abs(pix.y) && pix.x >  abs(pix.z)) {
    colorFromTex( pix.z,  pix.y,  pix.x, texRight);
    return;
  }

  if (pix.y < -abs(pix.x) && pix.y < -abs(pix.z)) {
    colorFromTex( pix.x, -pix.z, -pix.y, texBottom);
    return;
  }

  if (pix.y >  abs(pix.x) && pix.y >  abs(pix.z)) {
    colorFromTex( pix.x,  pix.z,  pix.y, texTop);
    return;
  }

  if (pix.z >  abs(pix.x) && pix.z >  abs(pix.y)) {
    colorFromTex(-pix.x,  pix.y,  pix.z, texBack);
    return;
  }

  if (pix.z < -abs(pix.x) && pix.z < -abs(pix.y)) {
    colorFromTex( pix.x,  pix.y, -pix.z, texFront);
    return;
  }

  fragColor = vec4(0.6, 0.3, 0.1, 1);
}
)";

  GM_VINF("EquirectangularView", "Creating fragment shader");
  fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment_shader_id, 1, &fragment_shader_code, nullptr);
  glCompileShader(fragment_shader_id);

  GM_VINF("EquirectangularView", "Creating and linking program");
  program_id = glCreateProgram();
  glAttachShader(program_id, vertex_shader_id);
  glAttachShader(program_id, fragment_shader_id);
  glLinkProgram(program_id);
  glBindAttribLocation(program_id, 0, "in_Position");

  if (!GLUtils::check_shader_program(program_id))
    return;

  GM_VINF("EquirectangularView", "Creating vertex array");
  glGenVertexArrays(1, &vao_id);
  glBindVertexArray(vao_id);

  GM_VINF("EquirectangularView", "Creating and setting up array buffer");
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

void EquirectangularView::Impl::teardown() {
  is_functional = false;

  if (fb_id[0]) glDeleteFramebuffers(SIDE_COUNT, fb_id);
  if (tex_id[0]) glDeleteTextures(SIDE_COUNT, tex_id);
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

void EquirectangularView::Impl::renderFullPipeline(ViewSettings settings) {
  if (!is_setup)
    setup();
  if (!is_functional)
    return;

  glGetIntegerv(GL_VIEWPORT, viewport);

  for (size_t idx = 0; idx < SIDE_COUNT; ++idx)
    renderSide(settings, idx);

  GM_VINF("EquirectangularView", "finalizing");

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);

  for (size_t idx = 0; idx < SIDE_COUNT; ++idx) {
    glActiveTexture(GL_TEXTURE0 + idx);
    glBindTexture( GL_TEXTURE_2D, tex_id[idx]);
  }

  glUseProgram(program_id);
  {
    GLint tex_idx = 0;
    glUniform1i(glGetUniformLocation(program_id, "texLeft"),   tex_idx++);
    glUniform1i(glGetUniformLocation(program_id, "texRight"),  tex_idx++);
    glUniform1i(glGetUniformLocation(program_id, "texBottom"), tex_idx++);
    glUniform1i(glGetUniformLocation(program_id, "texTop"),    tex_idx++);
    glUniform1i(glGetUniformLocation(program_id, "texBack"),   tex_idx++);
    glUniform1i(glGetUniformLocation(program_id, "texFront"),  tex_idx++);
  }

  glBindVertexArray(vao_id);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  glBindVertexArray(0);

  glUseProgram(0);
  glDisableVertexAttribArray(0);

  for (size_t idx = 0; idx < SIDE_COUNT; ++idx) {
    glActiveTexture(GL_TEXTURE0 + idx);
    glBindTexture( GL_TEXTURE_2D, 0);
  }
}

void EquirectangularView::Impl::renderSide(ViewSettings settings, size_t side) {

  glBindFramebuffer(GL_FRAMEBUFFER, fb_id[side]);
  glViewport(0, 0, resolution, resolution);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  Eigen::Vector3f pos = Eigen::Vector3f::Zero();
  Eigen::Quaternionf rot = Eigen::Quaternionf::Identity();

  if (settings.viewpoint) {
    pos = settings.viewpoint->getPosition();
    rot = settings.viewpoint->getOrientation();
  }

  Camera camera;
  camera.setPose(pos, rot * side_orientation[side]);

  for (auto renderer : settings.renderers)
    renderer->render(camera);
}

void EquirectangularView::setCubeMapResolution(int res) {
  _impl->resolution = res;
}

END_NAMESPACE_GMGRAPHICS;
