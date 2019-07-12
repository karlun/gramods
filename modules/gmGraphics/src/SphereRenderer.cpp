
#include <gmGraphics/SphereRenderer.hh>

#include <gmGraphics/GLUtils.hh>

#include <gmCore/Console.hh>
#include <gmCore/RunOnce.hh>

#include <GL/glew.h>
#include <GL/gl.h>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE(SphereRenderer);
GM_OFI_PARAM(SphereRenderer, radius, float, SphereRenderer::setRadius);
GM_OFI_PARAM(SphereRenderer, position, Eigen::Vector3f, SphereRenderer::setPosition);
GM_OFI_PARAM(SphereRenderer, orientation, Eigen::Quaternionf, SphereRenderer::setOrientation);
GM_OFI_POINTER(SphereRenderer, texture, gmGraphics::Texture, SphereRenderer::setTexture);
GM_OFI_POINTER(SphereRenderer, coordinatesMapper, gmGraphics::CoordinatesMapper, SphereRenderer::setCoordinatesMapper);

struct SphereRenderer::Impl {

  ~Impl();

  void setup();
  std::string createFragmentCode();
  void render(Camera camera);
  void teardown();

  static const std::string vertex_shader_code;
  static const std::string fragment_template_code;
  static const std::string mapper_pattern;

  GLuint vertex_shader_id = 0;
  GLuint fragment_shader_id = 0;
  GLuint program_id = 0;
  GLuint vao_id = 0;
  GLuint vbo_id[2] = { 0, 0 };

  GLuint vertex_count = 0;

  float radius = 10;
  Eigen::Vector3f position = Eigen::Vector3f::Zero();
  Eigen::Quaternionf orientation = Eigen::Quaternionf::Identity();

  std::shared_ptr<Texture> texture;
  std::shared_ptr<CoordinatesMapper> mapper;

  bool is_setup = false;
  bool is_functional = false;
};

SphereRenderer::SphereRenderer()
  : _impl(std::make_unique<Impl>()) {}

void SphereRenderer::render(Camera camera) {
  _impl->render(camera);
}

const std::string SphereRenderer::Impl::vertex_shader_code = R"lang=glsl(
#version 330 core

uniform mat4 Mp;
uniform mat4 Mv;
uniform mat4 Mm;

in vec4 vertex_position;
in vec3 tex_position;

out vec3 tex_pos;

void main() {
  gl_Position = Mp * Mv * Mm * vertex_position;
  tex_pos = tex_position;
}
)lang=glsl";

const std::string SphereRenderer::Impl::mapper_pattern = "MAPPER;";

const std::string SphereRenderer::Impl::fragment_template_code = R"lang=glsl(
#version 330 core

uniform sampler2D tex;

in vec3 tex_pos;
out vec4 fragColor;

MAPPER;

void main() {
  vec2 v_uv;
  bool good = mapTo2D(tex_pos, v_uv);
  if (good)
    fragColor = vec4(texture(tex, 0.5 + 0.5 * v_uv).rgb, 1);
  else
    fragColor = vec4(0, 0, 0, 0);
}
)lang=glsl";

void SphereRenderer::Impl::setup() {
  is_setup = true;

  if (!mapper) {
    GM_RUNONCE(GM_ERR("SphereRenderer", "No coordinate mapper specified."));
    return;
  }

  std::vector<GLfloat> vertices;
  std::vector<GLfloat> tcoords;

#define ROW_COUNT 32
#define COLUMN_COUNT 64

  for (size_t row = 0; row < ROW_COUNT; ++row) {

    double phi0 = gramods_PI * (double(row    ) / double(ROW_COUNT) - 0.5);
    double ry0 = sin(phi0);
    double rxz0 = cos(phi0);

    double phi1 = gramods_PI * (double(row + 1) / double(ROW_COUNT) - 0.5);
    double ry1 = sin(phi1);
    double rxz1 = cos(phi1);

    for (size_t column = 0; column < COLUMN_COUNT; ++column) {

      double theta0 = 2 * gramods_PI * double(column    ) / double(COLUMN_COUNT);
      double rx0 = sin(theta0);
      double rz0 = cos(theta0);

      double theta1 = 2 * gramods_PI * double(column + 1) / double(COLUMN_COUNT);
      double rx1 = sin(theta1);
      double rz1 = cos(theta1);

      if (row + 1 != ROW_COUNT) {
        tcoords.push_back(rx0 * rxz0); vertices.push_back(radius * tcoords.back());
        tcoords.push_back(       ry0); vertices.push_back(radius * tcoords.back());
        tcoords.push_back(rz0 * rxz0); vertices.push_back(radius * tcoords.back());
        tcoords.push_back(rx0 * rxz1); vertices.push_back(radius * tcoords.back());
        tcoords.push_back(       ry1); vertices.push_back(radius * tcoords.back());
        tcoords.push_back(rz0 * rxz1); vertices.push_back(radius * tcoords.back());
        tcoords.push_back(rx1 * rxz1); vertices.push_back(radius * tcoords.back());
        tcoords.push_back(       ry1); vertices.push_back(radius * tcoords.back());
        tcoords.push_back(rz1 * rxz1); vertices.push_back(radius * tcoords.back());
      }
      if (row != 0) {
        tcoords.push_back(rx0 * rxz0); vertices.push_back(radius * tcoords.back());
        tcoords.push_back(       ry0); vertices.push_back(radius * tcoords.back());
        tcoords.push_back(rz0 * rxz0); vertices.push_back(radius * tcoords.back());
        tcoords.push_back(rx1 * rxz1); vertices.push_back(radius * tcoords.back());
        tcoords.push_back(       ry1); vertices.push_back(radius * tcoords.back());
        tcoords.push_back(rz1 * rxz1); vertices.push_back(radius * tcoords.back());
        tcoords.push_back(rx1 * rxz0); vertices.push_back(radius * tcoords.back());
        tcoords.push_back(       ry0); vertices.push_back(radius * tcoords.back());
        tcoords.push_back(rz1 * rxz0); vertices.push_back(radius * tcoords.back());
      }
    }
  }

  assert(tcoords.size() == vertices.size());
  assert(tcoords.size() % 3 == 0);
  vertex_count = vertices.size();

  GM_VINF("SphereRenderer", "Creating vertex shader");
  vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
  const char *vert_strs[] = { vertex_shader_code.c_str() };
  glShaderSource(vertex_shader_id, 1, vert_strs, nullptr);
  glCompileShader(vertex_shader_id);

  GM_VINF("SphereRenderer", "Creating fragment shader");
  fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);
  std::string frag_str = createFragmentCode();
  const char *frag_strs[] = { frag_str.c_str() };
  glShaderSource(fragment_shader_id, 1, frag_strs, nullptr);
  glCompileShader(fragment_shader_id);

  GM_VINF("SphereRenderer", "Creating and linking program");
  program_id = glCreateProgram();
  glAttachShader(program_id, vertex_shader_id);
  glAttachShader(program_id, fragment_shader_id);
  glLinkProgram(program_id);
  glBindAttribLocation(program_id, 0, "in_Position");

  if (!GLUtils::check_shader_program(program_id))
    return;

  GM_VINF("SphereRenderer", "Creating vertex array");
  glGenVertexArrays(1, &vao_id);
  glBindVertexArray(vao_id);

  GM_VINF("SphereRenderer", "Creating and setting up array buffer");
  glGenBuffers(2, vbo_id);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_id[0]);
  glBufferData(GL_ARRAY_BUFFER,
               vertices.size() * sizeof(GLfloat),
               &vertices[0], GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_id[1]);
  glBufferData(GL_ARRAY_BUFFER,
               vertices.size() * sizeof(GLfloat),
               &tcoords[0], GL_STATIC_DRAW);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  GM_INF("SphereRenderer", "initialized");
  is_functional = true;
}

std::string SphereRenderer::Impl::createFragmentCode() {
  assert(mapper);
  assert(fragment_template_code.find(mapper_pattern) != std::string::npos);

  std::string mapper_code = mapper->getMapperCode();
  std::string fragment_code = fragment_template_code;

  fragment_code.replace(fragment_code.find(mapper_pattern),
                        mapper_pattern.length(),
                        mapper_code);

  return fragment_code;
}

void SphereRenderer::Impl::render(Camera camera) {

  if (!texture)
    GM_RUNONCE(GM_WRN("SphereRenderer", "No texture to render"));

  if (!is_setup)
    setup();
  if (!is_functional)
    return;

  GLuint tex_id = 0;
  if (texture) {
    texture->update();
    tex_id = texture->getGLTextureID();
  }

  GM_VINF("SphereRenderer", "rendering");

  Eigen::Affine3f Mm = Eigen::Translation3f(position) * orientation;
  Eigen::Affine3f Mv = camera.getViewMatrix();
  float near = std::max(1e-3f * radius,
                        0.99f * (-(Mv * Mm).translation().z() - radius));
  float far = 1.01f * (-(Mv * Mm).translation().z() + radius);
  Eigen::Matrix4f Mp = camera.getProjectionMatrix(near, far);

  glEnable(GL_DEPTH_TEST);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, tex_id);

  glUseProgram(program_id);
  glUniformMatrix4fv(glGetUniformLocation(program_id, "Mp"),  1, false, Mp.data());
  glUniformMatrix4fv(glGetUniformLocation(program_id, "Mv"),  1, false, Mv.data());
  glUniformMatrix4fv(glGetUniformLocation(program_id, "Mm"),  1, false, Mm.data());
  glUniform1i(glGetUniformLocation(program_id, "tex"), 0);
  mapper->setMapperUniforms(program_id);

  glBindVertexArray(vao_id);
  glDrawArrays(GL_TRIANGLES, 0, vertex_count);
  glBindVertexArray(0);

  glUseProgram(0);

  glBindTexture(GL_TEXTURE_2D, 0);

  GM_VVINF("SphereRenderer", "Done rendering");
}

SphereRenderer::Impl::~Impl() {
  teardown();
}

void SphereRenderer::Impl::teardown() {
  is_functional = false;

  if (program_id) glDeleteProgram(program_id);
  if (vertex_shader_id) glDeleteShader(vertex_shader_id);
  if (fragment_shader_id) glDeleteShader(fragment_shader_id);
  if (vao_id) glDeleteVertexArrays(1, &vao_id);
  if (vbo_id[0]) glDeleteBuffers(2, vbo_id);

  vertex_shader_id = 0;
  fragment_shader_id = 0;
  program_id = 0;
  vao_id = 0;
  vbo_id[0] = 0;

  is_setup = false;
}

void SphereRenderer::setRadius(float r) {
  _impl->radius = r;
}

void SphereRenderer::setPosition(Eigen::Vector3f p) {
  _impl->position = p;
}

void SphereRenderer::setOrientation(Eigen::Quaternionf q) {
  _impl->orientation = q;
}

void SphereRenderer::setTexture(std::shared_ptr<Texture> tex) {
  _impl->texture = tex;
}

void SphereRenderer::setCoordinatesMapper(std::shared_ptr<CoordinatesMapper> mapper) {
  _impl->mapper = mapper;
}

END_NAMESPACE_GMGRAPHICS;
