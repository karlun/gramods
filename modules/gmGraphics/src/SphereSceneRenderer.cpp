
#if 1
#include <gmGraphics/SphereSceneRenderer.hh>

#include <gmGraphics/GLUtils.hh>
#include <gmCore/MathConstants.hh>

#include <GL/glew.h>
#include <GL/gl.h>

#include <unordered_map>
#include <cmath>
#include <algorithm>
#include <limits>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE_SUB(SphereSceneRenderer, Renderer);
GM_OFI_PARAM2(SphereSceneRenderer, sphereRadius, float, setSphereRadius);
GM_OFI_PARAM2(SphereSceneRenderer, sphereSetRadius, float, setSphereSetRadius);
GM_OFI_PARAM2(SphereSceneRenderer, fillRatio, float, setFillRatio);
GM_OFI_PARAM2(SphereSceneRenderer, color, Eigen::Vector3f, setColor);

#define N_VERTICES 108

struct SphereSceneRenderer::Impl {

  ~Impl();

  void setup();
  void render(const Camera &camera, const Eigen::Affine3f &Mm);
  void getNearFar(const Camera &camera,
                  const Eigen::Affine3f &Mm,
                  float &near,
                  float &far);
  void teardown();

  GLuint vertex_shader_id = 0;
  GLuint fragment_shader_id = 0;
  GLuint program_id = 0;
  GLuint vao_id = 0;
  GLuint vbo_id = 0;

  float sphere_radius = 0.1f;
  float sphere_set_radius = 1.0f;
  float fill_ratio = 0.5;
  Eigen::Vector3f position = Eigen::Vector3f::Zero();
  Eigen::Vector3f color = Eigen::Vector3f::Ones();
  std::vector<Eigen::Vector3f> set_positions;
  std::vector<uint32_t> indices;

  bool is_setup = false;
  bool is_functional = false;
};

SphereSceneRenderer::SphereSceneRenderer()
  : _impl(std::make_unique<Impl>()) {}

void SphereSceneRenderer::render(const Camera &camera,
                                 const Eigen::Affine3f &Mm) {
  if (!eyes.empty() && eyes.count(camera.getEye()) == 0) return;
  _impl->render(camera, Mm);
}

namespace {
  const char * vertex_shader_code = R"lang=glsl(
#version 330 core

uniform mat4 Mp;
uniform mat4 Mv;
uniform mat4 Mm;

uniform float radius;

vec3 light1 = vec3(0.5773, 0.5773, -0.5773);
vec3 light2 = vec3(0, 0.7071, 0.7071);

in vec4 vertex_position;

out vec3 normal;
out vec3 local_light1;
out vec3 local_light2;

void main() {
  vec4 pos = vertex_position * vec4(radius, radius, radius, 1);
  gl_Position = Mp * Mv * Mm * pos;

  local_light1 = normalize(mat3(Mv) * light1);
  local_light2 = normalize(mat3(Mv) * light1);

  normal = normalize(mat3(Mv * Mm) * vertex_position.xyz);
}
)lang=glsl";

  const char * fragment_shader_code = R"lang=glsl(
#version 330 core

uniform vec3 color;

in vec3 normal;
in vec3 local_light1;
in vec3 local_light2;

out vec4 fragColor;

void main() {
  float diffuse = 0.2 +
    max(0, dot(normal, local_light1)) +
    max(0, dot(normal, local_light2));
  fragColor = vec4(diffuse * color.rgb, 1);
}
)lang=glsl";

uint32_t getPtIdx(std::vector<GLfloat> &vertices,
                      std::unordered_map<uint64_t, uint32_t> &new_pt_idx,
                      uint32_t i0,
                      uint32_t i1) {

  uint64_t iH01 = i0 < i1 ? i0 : i1;
  iH01 <<= 32;
  iH01 += i0 < i1 ? i1 : i0;

  auto it = new_pt_idx.find(iH01);
  if (it != new_pt_idx.end()) return it->second;

  Eigen::Vector3f p0(vertices[3 * i0 + 0],
                     vertices[3 * i0 + 1],
                     vertices[3 * i0 + 2]);
  Eigen::Vector3f p1(vertices[3 * i1 + 0],
                     vertices[3 * i1 + 1],
                     vertices[3 * i1 + 2]);
  Eigen::Vector3f p2 = (p0 + p1).normalized();

  vertices.push_back(p2[0]);
  vertices.push_back(p2[1]);
  vertices.push_back(p2[2]);

  if (vertices.size() / 3 > std::numeric_limits<uint32_t>::max())
    throw std::runtime_error("cannot refine polyhedron to more than 255 vertices");

  new_pt_idx[iH01] = vertices.size() / 3 - 1;
  return new_pt_idx[iH01];
}

void refine_polyhedron(std::vector<GLfloat> &vertices,
                       std::vector<uint32_t> &indices) {
  assert(vertices.size()%3 == 0);
  assert(indices.size()%3 == 0);

  std::vector<uint32_t> new_indices;
  new_indices.reserve(4 * indices.size());

  std::unordered_map<uint64_t, uint32_t> new_pt_idx;

  for (size_t idx = 0; idx < indices.size(); idx += 3) {
    uint32_t i0 = indices[idx + 0];
    uint32_t i1 = indices[idx + 1];
    uint32_t i2 = indices[idx + 2];

    uint32_t i01 = getPtIdx(vertices, new_pt_idx, i0, i1);
    uint32_t i02 = getPtIdx(vertices, new_pt_idx, i0, i2);
    uint32_t i12 = getPtIdx(vertices, new_pt_idx, i1, i2);

    new_indices.push_back(i0);
    new_indices.push_back(i01);
    new_indices.push_back(i02);

    new_indices.push_back(i1);
    new_indices.push_back(i12);
    new_indices.push_back(i01);

    new_indices.push_back(i2);
    new_indices.push_back(i02);
    new_indices.push_back(i12);

    new_indices.push_back(i01);
    new_indices.push_back(i12);
    new_indices.push_back(i02);

  }

  indices.swap(new_indices);
}

Eigen::Vector3f getMidPt(const std::vector<GLfloat> vertices,
                         uint32_t i0,
                         uint32_t i1,
                         uint32_t i2) {
  Eigen::Vector3f p0(vertices[3 * i0 + 0],
                     vertices[3 * i0 + 1],
                     vertices[3 * i0 + 2]);
  Eigen::Vector3f p1(vertices[3 * i1 + 0],
                     vertices[3 * i1 + 1],
                     vertices[3 * i1 + 2]);
  Eigen::Vector3f p2(vertices[3 * i2 + 0],
                     vertices[3 * i2 + 1],
                     vertices[3 * i2 + 2]);
  return (p0 + p1 + p2).normalized();
}
}

void SphereSceneRenderer::Impl::setup() {
  is_setup = true;

  std::vector<GLfloat> vertices =
    { -1.f, 0.f, 0.f,
      1.f,  0.f, 0.f,
      0.f, -1.f, 0.f,
      0.f,  1.f, 0.f,
      0.f,  0.f, -1.f,
      0.f,  0.f,  1.f };
  
  indices = { 2, 5, 0, 2, 1, 5, 2, 4, 1, 2, 0, 4,
              3, 0, 5, 3, 5, 1, 3, 1, 4, 3, 4, 0 };

  float necessary_spheres = fill_ratio * sphere_set_radius / sphere_radius;
  size_t refs =
      (size_t)(std::max(0.f, std::min(14.f, std::log2(necessary_spheres) - 2)));

  for (size_t idx = 0; idx < refs; ++idx)
    refine_polyhedron(vertices, indices);

  for (size_t idx = 0; idx < vertices.size(); idx += 3)
    set_positions.push_back(Eigen::Vector3f(vertices[idx + 0],
                                            vertices[idx + 1],
                                            vertices[idx + 2]));

  for (size_t idx = 0; idx < indices.size(); idx += 3)
    set_positions.push_back(getMidPt(vertices,
                                     indices[idx + 0],
                                     indices[idx + 1],
                                     indices[idx + 2]));

  for (int idx = refs; idx < 5; ++idx)
    refine_polyhedron(vertices, indices);

  GM_DBG2("SphereSceneRenderer", "Creating vertex shader");
  vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertex_shader_id, 1, &vertex_shader_code, nullptr);
  glCompileShader(vertex_shader_id);

  GM_DBG2("SphereSceneRenderer", "Creating fragment shader");
  fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment_shader_id, 1, &fragment_shader_code, nullptr);
  glCompileShader(fragment_shader_id);

  GM_DBG2("SphereSceneRenderer", "Creating and linking program");
  program_id = glCreateProgram();
  glAttachShader(program_id, vertex_shader_id);
  glAttachShader(program_id, fragment_shader_id);
  glLinkProgram(program_id);
  glBindAttribLocation(program_id, 0, "in_Position");

  if (!GLUtils::check_shader_program(program_id))
    return;

  GM_DBG2("SphereSceneRenderer", "Creating vertex array");
  glGenVertexArrays(1, &vao_id);
  glBindVertexArray(vao_id);

  GM_DBG2("SphereSceneRenderer", "Creating and setting up array buffer");
  glGenBuffers(1, &vbo_id);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
  glBufferData(GL_ARRAY_BUFFER,
               vertices.size() * sizeof(GLfloat),
               &vertices[0], GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

  glEnableVertexAttribArray(0);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  GM_DBG1("SphereSceneRenderer", "initialized");
  is_functional = true;
}

void SphereSceneRenderer::getNearFar(const Camera &camera,
                                     const Eigen::Affine3f &Mm,
                                     float &near,
                                     float &far) {
  if (!eyes.empty() && eyes.count(camera.getEye()) == 0) return;
  _impl->getNearFar(camera, Mm, near, far);
}

void SphereSceneRenderer::Impl::getNearFar(const Camera &camera,
                                           const Eigen::Affine3f &Mm,
                                           float &near,
                                           float &far) {

  Eigen::Affine3f Mv = camera.getViewMatrix();

  near =
    -(Mv * Mm).translation().z()
    - sphere_set_radius - sphere_radius;

  far =
    -(Mv * Mm).translation().z()
    + sphere_set_radius + sphere_radius;
}

void SphereSceneRenderer::Impl::render(const Camera &camera, const Eigen::Affine3f &Mm) {

  if (!is_setup)
    setup();
  if (!is_functional)
    return;

  GM_DBG2("SphereSceneRenderer", "rendering");

  Eigen::Affine3f Mv = camera.getViewMatrix();
  Eigen::Matrix4f Mp = camera.getProjectionMatrix();

  glUseProgram(program_id);
  glUniformMatrix4fv(glGetUniformLocation(program_id, "Mp"),  1, false, Mp.data());
  glUniformMatrix4fv(glGetUniformLocation(program_id, "Mv"),  1, false, Mv.matrix().data());
  auto Mm_id = glGetUniformLocation(program_id, "Mm");
  auto radius_id = glGetUniformLocation(program_id, "radius");
  auto color_id = glGetUniformLocation(program_id, "color");

  glUniform3fv(color_id, 1, color.data());
  glUniform1f(radius_id, sphere_radius);

  glBindVertexArray(vao_id);

  for (auto pos : set_positions) {

    Eigen::Affine3f cMm = Mm;
    cMm *= Eigen::Translation3f(position[0] + pos[0],
                                position[1] + pos[1],
                                position[2] + pos[2]);
    glUniformMatrix4fv(Mm_id, 1, false, cMm.data());
    glDrawElements(GL_TRIANGLES, indices.size(),  GL_UNSIGNED_INT, indices.data());
  }

  glBindVertexArray(0);
  glUseProgram(0);

  GM_DBG3("SphereSceneRenderer", "Done rendering");
}

SphereSceneRenderer::Impl::~Impl() {
  teardown();
}

void SphereSceneRenderer::Impl::teardown() {
  is_functional = false;

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

void SphereSceneRenderer::setSphereRadius(float d) {
  _impl->sphere_radius = d;
}

void SphereSceneRenderer::setSphereSetRadius(float d) {
  _impl->sphere_set_radius = d;
}

void SphereSceneRenderer::setFillRatio(float r) {
  _impl->fill_ratio = r;
}

void SphereSceneRenderer::setColor(Eigen::Vector3f c) {
  _impl->color = c;
}

END_NAMESPACE_GMGRAPHICS;
#endif
