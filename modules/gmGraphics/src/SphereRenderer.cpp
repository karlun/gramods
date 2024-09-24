
#include <gmGraphics/SphereRenderer.hh>

#include <gmGraphics/GLUtils.hh>

#include <gmCore/Console.hh>
#include <gmCore/RunOnce.hh>
#include <gmCore/MathConstants.hh>

#include <GL/glew.h>
#include <GL/gl.h>

#include <limits>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE_SUB(SphereRenderer, Renderer);
GM_OFI_PARAM2(SphereRenderer, radius, float, setRadius);
GM_OFI_POINTER2(SphereRenderer, texture, gmGraphics::TextureInterface, setTexture);
GM_OFI_POINTER2(SphereRenderer, coordinatesMapper, gmGraphics::CoordinatesMapper, setCoordinatesMapper);

struct SphereRenderer::Impl {

  ~Impl();

  void setup();
  std::string createFragmentCode();
  void render(const Camera &camera, const Eigen::Affine3f &Mm);
  void getNearFar(const Camera &camera,
                  const Eigen::Affine3f &Mm,
                  float &near,
                  float &far);
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

  std::shared_ptr<TextureInterface> texture;
  std::shared_ptr<CoordinatesMapper> mapper;

  bool is_setup = false;
  bool is_functional = false;
};

SphereRenderer::SphereRenderer()
  : _impl(std::make_unique<Impl>()) {}

void SphereRenderer::render(const Camera &camera, const Eigen::Affine3f &Mm) {
  if (!eyes.empty() && eyes.count(camera.getEye()) == 0) return;
  _impl->render(camera, Mm);
}

void SphereRenderer::getNearFar(const Camera &camera,
                                const Eigen::Affine3f &Mm,
                                float &near,
                                float &far) {
  if (!eyes.empty() && eyes.count(camera.getEye()) == 0) return;
  _impl->getNearFar(camera, Mm, near, far);
}

void SphereRenderer::Impl::getNearFar(const Camera &camera,
                                      const Eigen::Affine3f &Mm,
                                      float &near,
                                      float &far) {

  Eigen::Affine3f Mv = camera.getViewMatrix();

  float Z = (Mv * Mm).translation().z();
  near = -Z - radius;
  far = -Z + radius;
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
    fragColor = texture(tex, 0.5 + 0.5 * v_uv);
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

    double phi0 = GM_PI * (double(row    ) / double(ROW_COUNT) - 0.5);
    float ry0 = float(sin(phi0));
    float rxz0 = float(cos(phi0));

    double phi1 = GM_PI * (double(row + 1) / double(ROW_COUNT) - 0.5);
    float ry1 = float(sin(phi1));
    float rxz1 = float(cos(phi1));

    for (size_t column = 0; column < COLUMN_COUNT; ++column) {

      double theta0 = GM_2_PI * double(column    ) / double(COLUMN_COUNT);
      float rx0 = float(sin(theta0));
      float rz0 = float(cos(theta0));

      double theta1 = GM_2_PI * double(column + 1) / double(COLUMN_COUNT);
      float rx1 = float(sin(theta1));
      float rz1 = float(cos(theta1));

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

  GM_DBG2("SphereRenderer", "Creating vertex shader");
  vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
  const char *vert_strs[] = { vertex_shader_code.c_str() };
  glShaderSource(vertex_shader_id, 1, vert_strs, nullptr);
  glCompileShader(vertex_shader_id);

  GM_DBG2("SphereRenderer", "Creating fragment shader");
  fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);
  std::string frag_str = createFragmentCode();
  const char *frag_strs[] = { frag_str.c_str() };
  glShaderSource(fragment_shader_id, 1, frag_strs, nullptr);
  glCompileShader(fragment_shader_id);

  GM_DBG2("SphereRenderer", "Creating and linking program");
  program_id = glCreateProgram();
  glAttachShader(program_id, vertex_shader_id);
  glAttachShader(program_id, fragment_shader_id);
  glLinkProgram(program_id);
  glBindAttribLocation(program_id, 0, "in_Position");

  if (!GLUtils::check_shader_program(program_id))
    return;

  GM_DBG2("SphereRenderer", "Creating vertex array");
  glGenVertexArrays(1, &vao_id);
  glBindVertexArray(vao_id);

  GM_DBG2("SphereRenderer", "Creating and setting up array buffer");
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

  GM_DBG1("SphereRenderer", "initialized");
  is_functional = true;
}

std::string SphereRenderer::Impl::createFragmentCode() {
  assert(mapper);
  assert(fragment_template_code.find(mapper_pattern) != std::string::npos);

  std::string mapper_code =
      mapper->getCommonCode() + "\n" + mapper->getTo2DCode();
  std::string fragment_code = fragment_template_code;

  fragment_code.replace(fragment_code.find(mapper_pattern),
                        mapper_pattern.length(),
                        mapper_code);

  return fragment_code;
}

void SphereRenderer::Impl::render(const Camera &camera,
                                  const Eigen::Affine3f &Mm) {

  if (!texture)
    GM_RUNONCE(GM_WRN("SphereRenderer", "No texture to render"));

  if (!is_setup)
    setup();
  if (!is_functional)
    return;

  GLuint tex_id = 0;
  if (texture) {
    tex_id = texture->updateTexture(camera.frame_number, camera.getEye());
  }

  GM_DBG2("SphereRenderer", "rendering");

  Eigen::Affine3f Mv = camera.getViewMatrix();
  Eigen::Matrix4f Mp = camera.getProjectionMatrix();

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, tex_id);

  glUseProgram(program_id);
  glUniformMatrix4fv(glGetUniformLocation(program_id, "Mp"),  1, false, Mp.data());
  glUniformMatrix4fv(glGetUniformLocation(program_id, "Mv"),  1, false, Mv.data());
  glUniformMatrix4fv(glGetUniformLocation(program_id, "Mm"),  1, false, Mm.data());
  glUniform1i(glGetUniformLocation(program_id, "tex"), 0);
  mapper->setCommonUniforms(program_id);
  mapper->setTo2DUniforms(program_id);

  glBindVertexArray(vao_id);
  glDrawArrays(GL_TRIANGLES, 0, vertex_count);
  glBindVertexArray(0);

  glUseProgram(0);

  glBindTexture(GL_TEXTURE_2D, 0);

  GM_DBG3("SphereRenderer", "Done rendering");
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

void SphereRenderer::setTexture(std::shared_ptr<TextureInterface> tex) {
  _impl->texture = tex;
}

void SphereRenderer::setCoordinatesMapper(std::shared_ptr<CoordinatesMapper> mapper) {
  _impl->mapper = mapper;
}

void SphereRenderer::traverse(Visitor *visitor) {
  if (auto obj = std::dynamic_pointer_cast<gmCore::Object>(_impl->texture))
    obj->accept(visitor);
  if (_impl->mapper) _impl->mapper->accept(visitor);
}

END_NAMESPACE_GMGRAPHICS;
