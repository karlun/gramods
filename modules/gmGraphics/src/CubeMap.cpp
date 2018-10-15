
#include <gmGraphics/CubeMap.hh>

#include <gmGraphics/GLUtils.hh>

#include <GL/glew.h>
#include <GL/gl.h>

#include <vector>
#include <Eigen/Eigen>

BEGIN_NAMESPACE_GMGRAPHICS;

struct CubeMap::Impl {

  Impl(std::string fragment_code)
    : fragment_code(fragment_code) {}

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

  std::string fragment_code;

  int resolution = 2048;
  bool use_linear = false;

  GLuint framebuffer_id[SIDE_COUNT] = { 0 };
  GLuint texture_id[SIDE_COUNT] = { 0 };
  GLuint depth_renderbuffer_id = 0;

  GLuint vertex_shader_id = 0;
  GLuint fragment_shader_id = 0;
  GLuint program_id = 0;
  GLuint vertexarray_id = 0;
  GLuint vertexbuffer_id = 0;

  GLint viewport[4] = { 0, 0, 0, 0 };

  void setup();
  void teardown();

  void renderFullPipeline(RendererDispatcher::ViewSettings settings);
  void renderSide(RendererDispatcher::ViewSettings settings, size_t side);

  Eigen::Vector3f position = Eigen::Vector3f::Zero();
};

CubeMap::CubeMap(std::string fragment_code) {
  _impl = new Impl(fragment_code);
}

CubeMap::~CubeMap() {
  delete _impl;
  _impl = nullptr;
}

void CubeMap::renderFullPipeline(RendererDispatcher::ViewSettings settings) {
  _impl->renderFullPipeline(settings);
}

void CubeMap::Impl::setup() {
  is_setup = true;
  is_functional = false;

  if (resolution != GLUtils::nextPowerOfTwo(resolution)) {
    GM_WRN("CubeMap", "Cube map resolution (" << resolution << ") is not an even power of two");
  }

  GM_VINF("CubeMap", "Creating buffers and textures");
  glGenFramebuffers(SIDE_COUNT, framebuffer_id);
  glGenTextures(SIDE_COUNT, texture_id);
  glGenRenderbuffers(1, &depth_renderbuffer_id);

  for (size_t idx = 0; idx < SIDE_COUNT; ++idx) {
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_id[idx]);
    glBindTexture(GL_TEXTURE_2D, texture_id[idx]);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, resolution, resolution, 0,GL_RGB, GL_UNSIGNED_BYTE, 0);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    if (use_linear) {
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    } else {
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    }

    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_id[idx], 0);

    glBindRenderbuffer(GL_RENDERBUFFER, depth_renderbuffer_id);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32F, resolution, resolution);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_renderbuffer_id);

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

  GM_VINF("CubeMap", "Creating vertex shader");
  vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertex_shader_id, 1, &vertex_shader_code, nullptr);
  glCompileShader(vertex_shader_id);

  GM_VINF("CubeMap", "Creating fragment shader");
  fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);
  const char *strs[] = { fragment_code.c_str() };
  glShaderSource(fragment_shader_id, 1, strs, nullptr);
  glCompileShader(fragment_shader_id);

  GM_VINF("CubeMap", "Creating and linking program");
  program_id = glCreateProgram();
  glAttachShader(program_id, vertex_shader_id);
  glAttachShader(program_id, fragment_shader_id);
  glLinkProgram(program_id);
  glBindAttribLocation(program_id, 0, "in_Position");

  if (!GLUtils::check_shader_program(program_id))
    return;

  GM_VINF("CubeMap", "Creating vertex array");
  glGenVertexArrays(1, &vertexarray_id);
  glBindVertexArray(vertexarray_id);

  GM_VINF("CubeMap", "Creating and setting up array buffer");
  glGenBuffers(1, &vertexbuffer_id);
  glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_id);
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

void CubeMap::Impl::teardown() {
  is_functional = false;

  if (framebuffer_id[0]) glDeleteFramebuffers(SIDE_COUNT, framebuffer_id);
  if (texture_id[0]) glDeleteTextures(SIDE_COUNT, texture_id);
  if (depth_renderbuffer_id) glDeleteRenderbuffers(1, &depth_renderbuffer_id);

  framebuffer_id[0] = 0;
  texture_id[0] = 0;
  depth_renderbuffer_id = 0;

  if (program_id) glDeleteProgram(program_id);
  if (vertex_shader_id) glDeleteShader(vertex_shader_id);
  if (fragment_shader_id) glDeleteShader(fragment_shader_id);
  if (vertexarray_id) glDeleteVertexArrays(1, &vertexarray_id);
  if (vertexbuffer_id) glDeleteBuffers(1, &vertexbuffer_id);

  vertex_shader_id = 0;
  fragment_shader_id = 0;
  program_id = 0;
  vertexarray_id = 0;
  vertexbuffer_id = 0;

  is_setup = false;
}

GLint CubeMap::getProgram() {
  return _impl->program_id;
}

void CubeMap::Impl::renderFullPipeline(RendererDispatcher::ViewSettings settings) {
  if (!is_setup)
    setup();
  if (!is_functional)
    return;

  glGetIntegerv(GL_VIEWPORT, viewport);

  for (size_t idx = 0; idx < SIDE_COUNT; ++idx)
    renderSide(settings, idx);

  GM_VINF("CubeMap", "finalizing");

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);

  for (size_t idx = 0; idx < SIDE_COUNT; ++idx) {
    glActiveTexture(GL_TEXTURE0 + idx);
    glBindTexture( GL_TEXTURE_2D, texture_id[idx]);
  }

  glUseProgram(program_id);
  {
    GLint texture_idx = 0;
    glUniform1i(glGetUniformLocation(program_id, "texLeft"),   texture_idx++);
    glUniform1i(glGetUniformLocation(program_id, "texRight"),  texture_idx++);
    glUniform1i(glGetUniformLocation(program_id, "texBottom"), texture_idx++);
    glUniform1i(glGetUniformLocation(program_id, "texTop"),    texture_idx++);
    glUniform1i(glGetUniformLocation(program_id, "texBack"),   texture_idx++);
    glUniform1i(glGetUniformLocation(program_id, "texFront"),  texture_idx++);
  }

  glBindVertexArray(vertexarray_id);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  glBindVertexArray(0);

  glUseProgram(0);
  glDisableVertexAttribArray(0);

  for (size_t idx = 0; idx < SIDE_COUNT; ++idx) {
    glActiveTexture(GL_TEXTURE0 + idx);
    glBindTexture( GL_TEXTURE_2D, 0);
  }
}

void CubeMap::Impl::renderSide(RendererDispatcher::ViewSettings settings,
                               size_t side) {

  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_id[side]);
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

void CubeMap::setCubeMapResolution(int res) {
  _impl->resolution = res;
}

void CubeMap::setLinearInterpolation(bool on) {
  _impl->use_linear = on;
}

END_NAMESPACE_GMGRAPHICS;
