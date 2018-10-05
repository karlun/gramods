
#include <gmGraphics/CubeSceneRenderer.hh>

#include <GL/glew.h>
#include <GL/gl.h>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE(CubeSceneRenderer);
GM_OFI_PARAM(CubeSceneRenderer, cubeSize, float, CubeSceneRenderer::setCubeSize);
GM_OFI_PARAM(CubeSceneRenderer, cubeSetSize, float, CubeSceneRenderer::setCubeSetSize);
GM_OFI_PARAM(CubeSceneRenderer, cubeSetCenter, gmTypes::float3, CubeSceneRenderer::setCubeSetCenter);

#define N_VERTICES 108

struct CubeSceneRenderer::Impl {

  ~Impl();

  void setup();
  void render(Camera camera);
  void teardown();

  GLuint vertex_shader_id = 0;
  GLuint fragment_shader_id = 0;
  GLuint program_id = 0;
  GLuint vao_id = 0;
  GLuint vbo_id[2] = { 0, 0 };

  float cube_size = 0.1;
  float cube_set_size = 1.0;
  Eigen::Vector3f cube_set_center;
  bool has_been_setup = false;
};

CubeSceneRenderer::CubeSceneRenderer()
  : _impl(std::make_unique<Impl>()) {}

void CubeSceneRenderer::render(Camera camera) {
  _impl->render(camera);
}

namespace {
  const char * vertex_shader_code = R"(
#version 330 core

uniform mat4 Mp;
uniform mat4 Mv;
uniform mat4 Mm;

in vec4 vertex_position;
in vec3 vertex_normal;

out vec3 normal;

void main() {
  gl_Position = Mp * Mv * Mm * vertex_position;
  normal = normalize(mat3(Mv * Mm) * vertex_normal);
}
)";

  const char * fragment_shader_code = R"(
#version 330 core

uniform vec3 color;

in vec3 normal;
vec3 lightDirection = vec3(1,0,0);

out vec4 fragColor;

void main() {
  float diffuse = max(0.5, dot(normal, lightDirection));
  fragColor = vec4(diffuse * color.rgb, 1);
}
)";
}

void CubeSceneRenderer::Impl::setup() {

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

  GM_VINF("CubeSceneRenderer", "Creating vertex shader");
  vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertex_shader_id, 1, &vertex_shader_code, nullptr);
  glCompileShader(vertex_shader_id);

  GM_VINF("CubeSceneRenderer", "Creating fragment shader");
  fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment_shader_id, 1, &fragment_shader_code, nullptr);
  glCompileShader(fragment_shader_id);

  GM_VINF("CubeSceneRenderer", "Creating and linking program");
  program_id = glCreateProgram();
  glAttachShader(program_id, vertex_shader_id);
  glAttachShader(program_id, fragment_shader_id);
  glLinkProgram(program_id);
  glBindAttribLocation(program_id, 0, "in_Position");

  {
    GLsizei msg_len;
    GLchar msg_data[1024];
    glGetProgramInfoLog(program_id, 1024, &msg_len, msg_data);
    msg_data[1023] = '\0';
    GM_INF("CubeSceneRenderer", "GL program status: " << msg_data);
  }

  GM_VINF("CubeSceneRenderer", "Creating vertex array");
  glGenVertexArrays(1, &vao_id);
  glBindVertexArray(vao_id);

  GM_VINF("CubeSceneRenderer", "Creating and setting up array buffer");
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

  GM_INF("CubeSceneRenderer", "initialized");
  has_been_setup = true;
}

void CubeSceneRenderer::Impl::render(Camera camera) {
  if (!has_been_setup) setup();
  GM_VINF("CubeSceneRenderer", "rendering");

  auto Mv = camera.getViewMatrix();
  float near = 0.1 * cube_size;
  float far =
    (Mv.translation() - cube_set_center).norm()
    + 0.5 * (cube_set_size + cube_size);
  auto Mp = camera.getProjectionMatrix(near, far);

  size_t N = (size_t)(cube_set_size / (3.0 * cube_size));
  float pD = N > 1 ? cube_set_size / (N - 1) : 0.f;
  float p0 = -0.5f * cube_set_size;

  glEnable(GL_DEPTH_TEST);

  glUseProgram(program_id);
  glUniformMatrix4fv(glGetUniformLocation(program_id, "Mp"),  1, false, Mp.data());
  glUniformMatrix4fv(glGetUniformLocation(program_id, "Mv"),  1, false, Mv.data());
  auto Mm_id = glGetUniformLocation(program_id, "Mm");
  auto color_id = glGetUniformLocation(program_id, "color");

  glBindVertexArray(vao_id);

  float step = (1.f/N);
  for (int idx_z = 0; idx_z < N; ++idx_z)
    for (int idx_y = 0; idx_y < N; ++idx_y)
      for (int idx_x = 0; idx_x < N; ++idx_x) {

        if (idx_x != 0 && idx_x != N - 1 &&
            idx_y != 0 && idx_y != N - 1 &&
            idx_z != 0 && idx_z != N - 1)
          continue;

        Eigen::Affine3f Mm = Eigen::Affine3f::Identity();
        Mm *= Eigen::Translation3f(cube_set_center[0] + p0 + pD * idx_x,
                                   cube_set_center[1] + p0 + pD * idx_y,
                                   cube_set_center[2] + p0 + pD * idx_z);
        Mm *= Eigen::Scaling(0.5f * cube_size);
        Mm *= Eigen::AngleAxis<float>
          (1.0 * step * idx_x +
           0.4 * step * idx_y +
           0.7 * step * idx_z,
           Eigen::Vector3f(idx_z + 1, idx_y, idx_x).normalized());
        glUniformMatrix4fv(Mm_id, 1, false, Mm.data());

        Eigen::Vector3f color(0.5 + 0.5 * step * idx_x,
                              0.5 + 0.5 * step * idx_y - 0.5,
                              0.5 + 0.5 * step * idx_z - 0.5);
        glUniform3fv(color_id, 1, color.data());

        glDrawArrays(GL_TRIANGLES, 0, N_VERTICES);
      }

  glBindVertexArray(0);
  glUseProgram(0);

  GM_VVINF("CubeSceneRenderer", "Done rendering");
}

CubeSceneRenderer::Impl::~Impl() {
  teardown();
}

void CubeSceneRenderer::Impl::teardown() {

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

  has_been_setup = false;
}

void CubeSceneRenderer::setCubeSize(float d) {
  _impl->cube_size = d;
}

void CubeSceneRenderer::setCubeSetSize(float d) {
  _impl->cube_set_size = d;
}

void CubeSceneRenderer::setCubeSetCenter(gmTypes::float3 c) {
  _impl->cube_set_center = Eigen::Vector3f(c[0], c[1], c[2]);
}

END_NAMESPACE_GMGRAPHICS;
