
#include <gmGraphics/CubeSceneRenderer.hh>

#include <gmGraphics/GLUtils.hh>
#include <gmCore/MathConstants.hh>
#include <gmCore/Updateable.hh>

#include <GL/glew.h>
#include <GL/gl.h>

#include <chrono>
#include <limits>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE_SUB(CubeSceneRenderer, Renderer);
GM_OFI_PARAM2(CubeSceneRenderer, cubeSize, float, setCubeSize);
GM_OFI_PARAM2(CubeSceneRenderer, cubeSetSize, float, setCubeSetSize);
GM_OFI_PARAM2(CubeSceneRenderer, animate, bool, setAnimate);

#define N_VERTICES 108

struct CubeSceneRenderer::Impl : gmCore::Updateable {

  ~Impl();

  void setup();
  void render(const Camera &camera, const Eigen::Affine3f &Mm);
  void getNearFar(const Camera &camera,
                  const Eigen::Affine3f &Mm,
                  float &near,
                  float &far);
  void teardown();

  void update(clock::time_point time, size_t frame) override;

  GLuint vertex_shader_id = 0;
  GLuint fragment_shader_id = 0;
  GLuint program_id = 0;
  GLuint vao_id = 0;
  GLuint vbo_id[2] = { 0, 0 };

  float cube_size = 0.1f;
  float cube_set_size = 1.f;

  bool animate = true;
  double animation_secs = 0.0;

  bool is_setup = false;
  bool is_functional = false;
};

CubeSceneRenderer::CubeSceneRenderer()
  : _impl(std::make_unique<Impl>()) {}

void CubeSceneRenderer::render(const Camera &camera,
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

vec3 light1 = vec3(0.5773, 0.5773, -0.5773);
vec3 light2 = vec3(0, 0.7071, 0.7071);

in vec4 vertex_position;
in vec3 vertex_normal;

out vec3 normal;
out vec3 local_light1;
out vec3 local_light2;

void main() {
  gl_Position = Mp * Mv * Mm * vertex_position;

  local_light1 = normalize(mat3(Mv) * light1);
  local_light2 = normalize(mat3(Mv) * light1);

  normal = normalize(mat3(Mv * Mm) * vertex_normal);
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
  float diffuse = 0.5 +
    max(0, 3.0 * dot(normal, local_light1)) +
    max(0, 1.0 * dot(normal, local_light2));
  vec3 rgb = min(vec3(1,1,1), diffuse * color.rgb);
  fragColor = vec4(rgb, 1);
}
)lang=glsl";
}

void CubeSceneRenderer::Impl::setup() {
  is_setup = true;

  std::vector<GLfloat> vertices;
  std::vector<GLfloat> normals;

  static const float vertX[] = { -1.f, +1.f, -1.f, +1.f, -1.f, +1.f, -1.f, +1.f };
  static const float vertY[] = { -1.f, -1.f, +1.f, +1.f, -1.f, -1.f, +1.f, +1.f };
  static const float vertZ[] = { -1.f, -1.f, -1.f, -1.f, +1.f, +1.f, +1.f, +1.f };
  static const float normX[] = { -1.f, +1.f,  0.f,  0.f,  0.f,  0.f };
  static const float normY[] = {  0.f,  0.f, -1.f, +1.f,  0.f,  0.f };
  static const float normZ[] = {  0.f,  0.f,  0.f,  0.f, -1.f, +1.f };

#define VERT(id,idS)                            \
  vertices.push_back(vertX[id]);                \
  vertices.push_back(vertY[id]);                \
  vertices.push_back(vertZ[id]);                \
  normals.push_back(normX[idS]);                \
  normals.push_back(normY[idS]);                \
  normals.push_back(normZ[idS]);

#define SIDE(id0,id1,id2,id3,idS)               \
  VERT(id0,idS);                                \
  VERT(id1,idS);                                \
  VERT(id2,idS);                                \
  VERT(id0,idS);                                \
  VERT(id2,idS);                                \
  VERT(id3,idS);

  SIDE(0, 1, 5, 4, 2);
  SIDE(0, 2, 3, 1, 4);
  SIDE(0, 4, 6, 2, 0);
  SIDE(7, 3, 2, 6, 3);
  SIDE(7, 5, 1, 3, 1);
  SIDE(7, 6, 4, 5, 5);

#undef SIDE
#undef VERT
  assert(vertices.size() == N_VERTICES);
  assert(normals.size() == N_VERTICES);

  GM_DBG2("CubeSceneRenderer", "Creating vertex shader");
  vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertex_shader_id, 1, &vertex_shader_code, nullptr);
  glCompileShader(vertex_shader_id);

  GM_DBG2("CubeSceneRenderer", "Creating fragment shader");
  fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment_shader_id, 1, &fragment_shader_code, nullptr);
  glCompileShader(fragment_shader_id);

  GM_DBG2("CubeSceneRenderer", "Creating and linking program");
  program_id = glCreateProgram();
  glAttachShader(program_id, vertex_shader_id);
  glAttachShader(program_id, fragment_shader_id);
  glLinkProgram(program_id);
  glBindAttribLocation(program_id, 0, "in_Position");

  if (!GLUtils::check_shader_program(program_id))
    return;

  GM_DBG2("CubeSceneRenderer", "Creating vertex array");
  glGenVertexArrays(1, &vao_id);
  glBindVertexArray(vao_id);

  GM_DBG2("CubeSceneRenderer", "Creating and setting up array buffer");
  glGenBuffers(2, vbo_id);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_id[0]);
  glBufferData(GL_ARRAY_BUFFER,
               vertices.size() * sizeof(GLfloat),
               &vertices[0], GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_id[1]);
  glBufferData(GL_ARRAY_BUFFER,
               vertices.size() * sizeof(GLfloat),
               &normals[0], GL_STATIC_DRAW);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  GM_DBG1("CubeSceneRenderer", "initialized");
  is_functional = true;
}

void CubeSceneRenderer::getNearFar(const Camera &camera,
                                   const Eigen::Affine3f &Mm,
                                   float &near,
                                   float &far) {
  if (!eyes.empty() && eyes.count(camera.getEye()) == 0) return;
  _impl->getNearFar(camera, Mm, near, far);
}

void CubeSceneRenderer::Impl::getNearFar(const Camera &camera,
                                         const Eigen::Affine3f &Mm,
                                         float &near,
                                         float &far) {

  Eigen::Affine3f Mv = camera.getViewMatrix();

  near =
    -(Mv * Mm).translation().z()
    - 0.87f * (cube_set_size + cube_size);

  far =
    -(Mv * Mm).translation().z()
    + 0.87f * (cube_set_size + cube_size);
}

void CubeSceneRenderer::Impl::render(const Camera &camera,
                                     const Eigen::Affine3f &Mm) {

  if (!is_setup)
    setup();
  if (!is_functional)
    return;

  GM_DBG2("CubeSceneRenderer", "rendering");

  Eigen::Affine3f Mv = camera.getViewMatrix();
  Eigen::Matrix4f Mp = camera.getProjectionMatrix();

  size_t N = (size_t)(cube_set_size / (3.f * cube_size));
  float pD = N > 1 ? cube_set_size / (N - 1) : 0.f;
  float p0 = -0.5f * cube_set_size;

  glUseProgram(program_id);
  glUniformMatrix4fv(glGetUniformLocation(program_id, "Mp"),  1, false, Mp.data());
  glUniformMatrix4fv(glGetUniformLocation(program_id, "Mv"),  1, false, Mv.matrix().data());
  auto Mm_id = glGetUniformLocation(program_id, "Mm");
  auto color_id = glGetUniformLocation(program_id, "color");

  glBindVertexArray(vao_id);

  static const clock::time_point start_time = clock::now();
  static const double rate = GM_PI;

  float step = (1.f/N);
  for (size_t idx_z = 0; idx_z < N; ++idx_z)
    for (size_t idx_y = 0; idx_y < N; ++idx_y)
      for (size_t idx_x = 0; idx_x < N; ++idx_x) {

        if (idx_x != 0 && idx_x != N - 1 &&
            idx_y != 0 && idx_y != N - 1 &&
            idx_z != 0 && idx_z != N - 1)
          continue;

        Eigen::Affine3f cMm = Mm;
        cMm *= Eigen::Translation3f(
            p0 + pD * idx_x, p0 + pD * idx_y, p0 + pD * idx_z);
        cMm *= Eigen::Scaling(0.5f * cube_size);
        cMm *= Eigen::AngleAxis<float>
          (1.0f * step * idx_x +
           0.4f * step * idx_y +
           0.7f * step * idx_z +
           (float)(rate * animation_secs),
           Eigen::Vector3f(idx_z + 1, idx_y, idx_x).normalized());
        glUniformMatrix4fv(Mm_id, 1, false, cMm.data());

        Eigen::Vector3f color(0.2f + 0.8f * step * idx_x,
                              0.2f + 0.8f * step * idx_y,
                              0.2f + 0.8f * step * idx_z);
        glUniform3fv(color_id, 1, color.data());

        glDrawArrays(GL_TRIANGLES, 0, N_VERTICES);
      }

  glBindVertexArray(0);
  glUseProgram(0);

  GM_DBG3("CubeSceneRenderer", "Done rendering");
}

CubeSceneRenderer::Impl::~Impl() {
  teardown();
}

void CubeSceneRenderer::Impl::teardown() {
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

void CubeSceneRenderer::Impl::update(clock::time_point time, size_t frame) {
  if (!animate) return;

  typedef std::chrono::duration<double, std::ratio<1>> d_seconds;

  static auto start_time = time;
  animation_secs =
      std::chrono::duration_cast<d_seconds>(time - start_time).count();
}

void CubeSceneRenderer::setCubeSize(float d) {
  _impl->cube_size = d;
}

void CubeSceneRenderer::setCubeSetSize(float d) {
  _impl->cube_set_size = d;
}

void CubeSceneRenderer::setAnimate(bool on) {
  _impl->animate = on;
}

END_NAMESPACE_GMGRAPHICS;
