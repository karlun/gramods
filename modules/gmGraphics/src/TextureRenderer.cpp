
#include <gmGraphics/TextureRenderer.hh>

#include <gmCore/Console.hh>
#include <gmCore/RunOnce.hh>

#define TEXTURE_IDX 0

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE(TextureRenderer);
GM_OFI_POINTER(TextureRenderer, texture, gmGraphics::TextureInterface, TextureRenderer::setTexture);

namespace {
  const char * vertex_shader_code = R"lang=glsl(
#version 330 core

in vec2 a_vertex;
out vec2 v_uv;

void main() {
  v_uv = a_vertex * 0.5 - 0.5;
  v_uv.y = 1 - v_uv.y;
  gl_Position = vec4(a_vertex, 0.0, 1.0);
}
)lang=glsl";

  const char * fragment_shader_code = R"lang=glsl(
#version 330 core

uniform sampler2D tex;

in vec2 v_uv;
out vec4 fragColor;

void main() {
  fragColor = texture(tex, v_uv);
}
)lang=glsl";
}

struct TextureRenderer::Impl {

  ~Impl();

  void render(TextureInterface *tex, Camera &camera);
  void setup();

  GLuint vertex_shader_id = 0;
  GLuint fragment_shader_id = 0;
  GLuint program_id = 0;
  GLuint vao_id = 0;
  GLuint vbo_id = 0;

  bool has_been_setup = false;
};

TextureRenderer::TextureRenderer()
  : _impl(new Impl) {}

void TextureRenderer::render(Camera camera, float near, float far) {
  _impl->render(texture.get(), camera);
}

void TextureRenderer::getNearFar(Camera camera, float &near, float &far) {
  near = far = -1;
}

void TextureRenderer::Impl::render(TextureInterface *texture, Camera &camera) {
  if (!texture) {
    GM_RUNONCE(GM_WRN("TextureRenderer", "No texture to render"));
    return;
  }

  if (!has_been_setup) setup();
  GM_VINF("TextureRenderer", "rendering");

  texture->update();

  if (!texture->getGLTextureID()) return;
  GLuint tex_id = texture->getGLTextureID();

  glDisable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glActiveTexture(GL_TEXTURE0 + TEXTURE_IDX);
  glBindTexture(GL_TEXTURE_2D, tex_id);
  glUseProgram(program_id);
  glUniform1i(glGetUniformLocation(program_id, "tex"), TEXTURE_IDX);

  glBindVertexArray(vao_id);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  glBindVertexArray(0);

  glUseProgram(0);
  glDisableVertexAttribArray(0);
  glBindTexture( GL_TEXTURE_2D, 0);
  GM_VVINF("TextureRenderer", "Done rendering");
}

void TextureRenderer::Impl::setup() {

  GM_VINF("TextureRenderer", "Creating vertex shader");
  vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertex_shader_id, 1, &vertex_shader_code, nullptr);
  glCompileShader(vertex_shader_id);

  GM_VINF("TextureRenderer", "Creating fragment shader");
  fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment_shader_id, 1, &fragment_shader_code, nullptr);
  glCompileShader(fragment_shader_id);

  GM_VINF("TextureRenderer", "Creating and linking program");
  program_id = glCreateProgram();
  glAttachShader(program_id, vertex_shader_id);
  glAttachShader(program_id, fragment_shader_id);
  glLinkProgram(program_id);
  glBindAttribLocation(program_id, 0, "in_Position");
    
  GM_VINF("TextureRenderer", "Creating vertex array");
  glGenVertexArrays(1, &vao_id);
  glBindVertexArray(vao_id);

  GM_VINF("TextureRenderer", "Creating and setting up array buffer");
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

  GM_INF("TextureRenderer", "initialized");
  has_been_setup = true;
}

TextureRenderer::Impl::~Impl() {
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
}


END_NAMESPACE_GMGRAPHICS;
