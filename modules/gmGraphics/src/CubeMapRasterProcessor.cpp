
#include <gmGraphics/CubeMapRasterProcessor.hh>

#include <gmGraphics/GLUtils.hh>
#include <gmCore/MathConstants.hh>

#include <GL/glew.h>
#include <GL/gl.h>

#include <vector>
#include <Eigen/Eigen>

BEGIN_NAMESPACE_GMGRAPHICS;

struct CubeMapRasterProcessor::Impl {

  ~Impl() {
    teardown();
  }

  static const size_t SIDE_COUNT = 6;

  Eigen::Quaternionf side_orientation[SIDE_COUNT] = {
      Eigen::Quaternionf(Eigen::Quaternionf::AngleAxisType(
          float(GM_PI_2), Eigen::Vector3f::UnitY())),
      Eigen::Quaternionf(Eigen::Quaternionf::AngleAxisType(
          float(-GM_PI_2), Eigen::Vector3f::UnitY())),
      Eigen::Quaternionf(Eigen::Quaternionf::AngleAxisType(
          float(-GM_PI_2), Eigen::Vector3f::UnitX())),
      Eigen::Quaternionf(Eigen::Quaternionf::AngleAxisType(
          float(GM_PI_2), Eigen::Vector3f::UnitX())),
      Eigen::Quaternionf(Eigen::Quaternionf::AngleAxisType(
          float(GM_PI), Eigen::Vector3f::UnitY())),
      Eigen::Quaternionf::Identity()};

  bool is_setup = false;
  bool is_functional = false;

  std::string fragment_code;

  int resolution = 2048;
  bool use_linear = false;
  GLenum pixel_format = GL_RGBA8;

  GLuint framebuffer_id[SIDE_COUNT] = { 0 };
  GLuint texture_id[SIDE_COUNT] = { 0 };
  GLuint depth_renderbuffer_id = 0;

  GLuint vertex_shader_id = 0;
  GLuint fragment_shader_id = 0;
  GLuint program_id = 0;
  GLuint vertexarray_id = 0;
  GLuint vertexbuffer_id = 0;

  void setup();
  void teardown();

  void renderFullPipeline(ViewBase::ViewSettings &settings,
                          Eigen::Vector3f pos,
                          Eigen::Quaternionf rot,
                          Eye eye,
                          bool make_square);
  void renderSide(ViewBase::ViewSettings &settings,
                  Eigen::Vector3f pos,
                  Eigen::Quaternionf rot,
                  Eye eye,
                  size_t side);

  bool spatial_cubemap = false;
  Eigen::Vector3f cubemap_position = Eigen::Vector3f::Zero();
  float cubemap_side = 1.0;
};

CubeMapRasterProcessor::CubeMapRasterProcessor()
  : _impl(new Impl()) {}

CubeMapRasterProcessor::~CubeMapRasterProcessor() {
  delete _impl;
  _impl = nullptr;
}

void CubeMapRasterProcessor::renderFullPipeline(
    ViewBase::ViewSettings &settings,
    Eigen::Vector3f pos,
    Eigen::Quaternionf rot,
    Eye eye,
    bool make_square) {
  _impl->renderFullPipeline(settings, pos, rot, eye, make_square);
}

void CubeMapRasterProcessor::Impl::setup() {
  is_setup = true;
  is_functional = false;

  if (resolution != GLUtils::nextPowerOfTwo(resolution)) {
    GM_WRN("CubeMapRasterProcessor", "Cube map resolution (" << resolution << ") is not an even power of two");
  }

  GM_DBG2("CubeMapRasterProcessor", "Creating buffers and textures");
  glGenFramebuffers(SIDE_COUNT, framebuffer_id);
  glGenTextures(SIDE_COUNT, texture_id);
  glGenRenderbuffers(1, &depth_renderbuffer_id);

  GLint previous_framebuffer;
  glGetIntegerv(GL_FRAMEBUFFER_BINDING, &previous_framebuffer);

  for (size_t idx = 0; idx < SIDE_COUNT; ++idx) {
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_id[idx]);
    glBindTexture(GL_TEXTURE_2D, texture_id[idx]);

    glTexImage2D(GL_TEXTURE_2D, 0, pixel_format, resolution, resolution, 0,
                 GL_RGB, GL_UNSIGNED_BYTE, 0);

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

    if (!GLUtils::check_framebuffer()) {
      glBindFramebuffer(GL_FRAMEBUFFER, previous_framebuffer);
      return;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, previous_framebuffer);
  }

  static const char * vertex_shader_code = R"lang=glsl(
#version 330 core

in vec2 a_vertex;
out vec2 pos;

void main() {
  pos = a_vertex;
  gl_Position = vec4(a_vertex, 0.0, 1.0);
}
)lang=glsl";

  GM_DBG2("CubeMapRasterProcessor", "Creating vertex shader");
  vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertex_shader_id, 1, &vertex_shader_code, nullptr);
  glCompileShader(vertex_shader_id);

  GM_DBG2("CubeMapRasterProcessor", "Creating fragment shader");
  fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);
  const char *strs[] = { fragment_code.c_str() };
  glShaderSource(fragment_shader_id, 1, strs, nullptr);
  glCompileShader(fragment_shader_id);

  GM_DBG2("CubeMapRasterProcessor", "Creating and linking program");
  program_id = glCreateProgram();
  glAttachShader(program_id, vertex_shader_id);
  glAttachShader(program_id, fragment_shader_id);
  glLinkProgram(program_id);
  glBindAttribLocation(program_id, 0, "in_Position");

  if (!GLUtils::check_shader_program(program_id))
    return;

  GM_DBG2("CubeMapRasterProcessor", "Creating vertex array");
  glGenVertexArrays(1, &vertexarray_id);
  glBindVertexArray(vertexarray_id);

  GM_DBG2("CubeMapRasterProcessor", "Creating and setting up array buffer");
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

void CubeMapRasterProcessor::Impl::teardown() {
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

GLint CubeMapRasterProcessor::getProgram() {
  return _impl->program_id;
}

void CubeMapRasterProcessor::Impl::renderFullPipeline(
    ViewBase::ViewSettings &settings,
    Eigen::Vector3f pos,
    Eigen::Quaternionf rot,
    Eye eye,
    bool make_square) {

  GLint previous_viewport[4] = { 0, 0, 0, 0 };
  glGetIntegerv(GL_VIEWPORT, previous_viewport);

  GLint previous_framebuffer;
  glGetIntegerv(GL_FRAMEBUFFER_BINDING, &previous_framebuffer);

  if (!is_setup)
    setup();
  if (!is_functional)
    return;
  if (settings.nodes.empty())
    return;

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  for (size_t idx = 0; idx < SIDE_COUNT; ++idx)
    renderSide(settings, pos, rot, eye, idx);

  GM_DBG2("CubeMapRasterProcessor", "finalizing");

  glBindFramebuffer(GL_FRAMEBUFFER, previous_framebuffer);

  if (make_square) {

    GLint x0 = previous_viewport[0];
    GLint y0 = previous_viewport[1];
    GLint width = previous_viewport[2];
    GLint height = previous_viewport[3];

    if (width > height) {
      glViewport(x0 + (width - height) / 2, y0, height, height);
    } else if (width < height) {
      glViewport(x0, y0 + (height - width)/2, width, width);
    } else {
      glViewport(x0, y0, width, height);
    }
  } else {
    glViewport(previous_viewport[0], previous_viewport[1],
               previous_viewport[2], previous_viewport[3]);
  }

  for (size_t idx = 0; idx < SIDE_COUNT; ++idx) {
    glActiveTexture(GLenum(GL_TEXTURE0 + idx));
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

  glDisable(GL_DEPTH_TEST);

  glBindVertexArray(vertexarray_id);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  glBindVertexArray(0);

  glUseProgram(0);
  glDisableVertexAttribArray(0);

  for (size_t idx = 0; idx < SIDE_COUNT; ++idx) {
    glActiveTexture(GLenum(GL_TEXTURE0 + idx));
    glBindTexture( GL_TEXTURE_2D, 0);
  }

  if (make_square)
    glViewport(previous_viewport[0], previous_viewport[1],
               previous_viewport[2], previous_viewport[3]);
}

void CubeMapRasterProcessor::Impl::renderSide
(ViewBase::ViewSettings &settings,
 Eigen::Vector3f pos,
 Eigen::Quaternionf rot,
 Eye eye,
 size_t side) {

  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_id[side]);
  glViewport(0, 0, resolution, resolution);

  glClearColor(0, 0, 0, 0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  Camera camera(settings.frame_number);
  camera.setPose(pos, rot * side_orientation[side]);
  camera.setEye(eye);

  if (spatial_cubemap) {

    Eigen::Vector3f O = pos - cubemap_position;
    float W = 0.5f * cubemap_side;

    switch (side) {
    case 0: // left
      camera.setClipPlanes((-W + O.z()) / (W + O.x()),
                           (+W + O.z()) / (W + O.x()),
                           (-W - O.y()) / (W + O.x()),
                           (+W - O.y()) / (W + O.x()));
      break;
    case 1: // right
      camera.setClipPlanes((-W - O.z()) / (W - O.x()),
                           (+W - O.z()) / (W - O.x()),
                           (-W - O.y()) / (W - O.x()),
                           (+W - O.y()) / (W - O.x()));
      break;
    case 2: // bottom
      camera.setClipPlanes((-W - O.x()) / (W + O.y()),
                           (+W - O.x()) / (W + O.y()),
                           (-W + O.z()) / (W + O.y()),
                           (+W + O.z()) / (W + O.y()));
      break;
    case 3: // top
      camera.setClipPlanes((-W - O.x()) / (W - O.y()),
                           (+W - O.x()) / (W - O.y()),
                           (-W - O.z()) / (W - O.y()),
                           (+W - O.z()) / (W - O.y()));
      break;
    case 4: // back
      camera.setClipPlanes((-W + O.x()) / (W - O.z()),
                           (+W + O.x()) / (W - O.z()),
                           (-W - O.y()) / (W - O.z()),
                           (+W - O.y()) / (W - O.z()));
      break;
    case 5: // front
      camera.setClipPlanes((-W - O.x()) / (W + O.z()),
                           (+W - O.x()) / (W + O.z()),
                           (-W - O.y()) / (W + O.z()),
                           (+W - O.y()) / (W + O.z()));
      break;
    default: // only six sides - this should not happen
      assert(0);
    }
  }

  settings.renderNodes(camera);
}

void CubeMapRasterProcessor::setFragmentCode(std::string code) {
  _impl->fragment_code = code;
}

void CubeMapRasterProcessor::setCubeMapResolution(int res) {
  _impl->resolution = res;
}

void CubeMapRasterProcessor::setPixelFormat(GLenum format) {
  _impl->pixel_format = format;
}

GLenum CubeMapRasterProcessor::getPixelFormat() {
  return _impl->pixel_format;
}

void CubeMapRasterProcessor::setLinearInterpolation(bool on) {
  _impl->use_linear = on;
}

void CubeMapRasterProcessor::setSpatialCubeMap(Eigen::Vector3f c, float side) {
  _impl->spatial_cubemap = true;
  _impl->cubemap_position = c;
  _impl->cubemap_side = side;
}

END_NAMESPACE_GMGRAPHICS;
