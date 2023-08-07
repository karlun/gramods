
#include <gmGraphics/RasterProcessor.hh>

#include <gmGraphics/GLUtils.hh>
#include <gmCore/Console.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

struct RasterProcessor::Impl {

  ~Impl();

  bool is_setup = false;
  bool is_functional = false;

  std::string vertex_shader_code = R"lang=glsl(
#version 330 core

in vec2 a_vertex;
out vec2 position;

void main() {
  gl_Position = vec4(a_vertex, 0.0, 1.0);
  position = a_vertex;
}
)lang=glsl";
  std::string fragment_shader_code = "";

  GLuint vertex_shader_id = 0;
  GLuint fragment_shader_id = 0;
  GLuint program_id = 0;
  GLuint vao_id = 0;
  GLuint vbo_id = 0;

  bool init();
  void teardown();

  void run();
};

RasterProcessor::RasterProcessor()
  : _impl(std::make_unique<RasterProcessor::Impl>()) {}

RasterProcessor::~RasterProcessor() {}

RasterProcessor::Impl::~Impl() {
  teardown();
}

void RasterProcessor::setVertexCode(std::string code) {
  if (_impl->is_setup)
    GM_WRN("RasterProcessor", "Setting code after initialization - new code is ignored.");
  _impl->vertex_shader_code = code;
}

void RasterProcessor::setFragmentCode(std::string code) {
  if (_impl->is_setup)
    GM_WRN("RasterProcessor", "Setting code after initialization - new code is ignored.");
  _impl->fragment_shader_code = code;
}

bool RasterProcessor::init() {
  return _impl->init();
}

void RasterProcessor::run() {
  _impl->run();
}

GLuint RasterProcessor::getProgramId() {
  return _impl->program_id;
}

bool RasterProcessor::Impl::init() {

  is_setup = true;
  is_functional = false;

  if (fragment_shader_code == "") {
    GM_ERR("RasterProcessor", "Fragment shader code not specified");
    return false;
  }

  GM_DBG2("RasterProcessor", "Creating vertex shader");
  vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
  {const char *p = vertex_shader_code.c_str();
    glShaderSource(vertex_shader_id, 1, &p, nullptr);}
  glCompileShader(vertex_shader_id);

  GM_DBG2("RasterProcessor", "Creating fragment shader");
  fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);
  {const char *p = fragment_shader_code.c_str();
    glShaderSource(fragment_shader_id, 1, &p, nullptr);}
  glCompileShader(fragment_shader_id);

  GM_DBG2("RasterProcessor", "Creating and linking program");
  program_id = glCreateProgram();
  glAttachShader(program_id, vertex_shader_id);
  glAttachShader(program_id, fragment_shader_id);
  glLinkProgram(program_id);
  glBindAttribLocation(program_id, 0, "in_Position");

  if (!GLUtils::check_shader_program(program_id))
    return false;

  GM_DBG2("RasterProcessor", "Creating vertex array");
  glGenVertexArrays(1, &vao_id);
  glBindVertexArray(vao_id);

  GM_DBG2("RasterProcessor", "Creating and setting up array buffer");
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
  return true;
}

void RasterProcessor::Impl::teardown() {
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

void RasterProcessor::Impl::run() {

  GM_DBG2("RasterProcessor", "run");

  glUseProgram(program_id);

  glBindVertexArray(vao_id);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  glBindVertexArray(0);

  glUseProgram(0);
  glDisableVertexAttribArray(0);
}


END_NAMESPACE_GMGRAPHICS;
