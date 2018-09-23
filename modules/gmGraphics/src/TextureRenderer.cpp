
#include <gmGraphics/TextureRenderer.hh>

#include <gmCore/Console.hh>

#include <glbinding/gl/enum.h>

#include <globjects/globjects.h>
#include <globjects/base/StaticStringSource.h>
#include <globjects/VertexAttributeBinding.h>
#include <globjects/Texture.h>

#include <glm/glm.hpp>


#define UNIFORM_TEXTURE_ID 0

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE(TextureRenderer);
GM_OFI_POINTER(TextureRenderer, texture, gmGraphics::Texture, TextureRenderer::setTexture);

namespace {
  const char * vertex_shader_code = R"(
#version 140
#extension GL_ARB_explicit_attrib_location : require

layout (location = 0) in vec2 a_vertex;
out vec2 v_uv;

void main()
{
    v_uv = a_vertex * 0.5 + 0.5;
    gl_Position = vec4(a_vertex, 0.0, 1.0);
}
)";

  const char * fragment_shader_code = R"(
#version 140
#extension GL_ARB_explicit_attrib_location : require
uniform sampler2D tex;
layout (location = 0) out vec4 fragColor;
in vec2 v_uv;
void main()
{
    fragColor = texture(tex, v_uv);
}
)";
}

struct TextureRenderer::_This {

  void render(Texture *tex, Camera &camera);
  void setup();

  std::unique_ptr<globjects::Program> program;
  std::unique_ptr<globjects::VertexArray> vao;
  std::unique_ptr<globjects::Shader> vertex_shader;
  std::unique_ptr<globjects::Shader> fragment_shader;
};

TextureRenderer::TextureRenderer()
  : _this(new _This) {}

void TextureRenderer::setup() {
  _this->setup();
}

void TextureRenderer::render(Camera camera) {
  _this->render(texture.get(), camera);
}

void TextureRenderer::_This::render(Texture *texture, Camera &camera) {
  GM_VINF("TextureRenderer", "rendering");

  texture->update();

  if (!texture->getGLTextureID()) return;
  std::unique_ptr<globjects::Texture> gl_texture =
    globjects::Texture::fromId(texture->getGLTextureID(), gl::GL_TEXTURE_2D);

  glActiveTexture(gl::GL_TEXTURE0);
  gl_texture->bind();

  program->use();
  vao->drawArrays(gl::GL_TRIANGLE_STRIP, 0, 4);
  program->release();

  gl_texture->unbind();
}

void TextureRenderer::_This::setup() {

  auto vertex_shader_str = globjects::Shader::sourceFromString(vertex_shader_code);
  vertex_shader =
    std::make_unique<globjects::Shader>
    (gl::GL_VERTEX_SHADER, vertex_shader_str.get());
  auto fragment_shader_str = globjects::Shader::sourceFromString(fragment_shader_code);
  fragment_shader =
    std::make_unique<globjects::Shader>
    (gl::GL_FRAGMENT_SHADER, fragment_shader_str.get());

  program = std::make_unique<globjects::Program>();
  program->attach(vertex_shader.get(), fragment_shader.get());

  static const std::array<glm::vec2, 4> raw
  {{    glm::vec2(+1.f,-1.f),
        glm::vec2(+1.f,+1.f),
        glm::vec2(-1.f,-1.f),
        glm::vec2(-1.f,+1.f)
        }};

  vao = std::make_unique<globjects::VertexArray>();

  auto buffer = new globjects::Buffer();
  buffer->setData(raw, gl::GL_STATIC_DRAW);

  auto binding = vao->binding(0);
  binding->setAttribute(0);
  binding->setBuffer(buffer, 0, sizeof(glm::vec2));
  binding->setFormat(2, gl::GL_FLOAT, gl::GL_FALSE, 0);
  vao->enable(0);

  program->setUniform("tex", UNIFORM_TEXTURE_ID);

  GM_INF("TextureRenderer", "initialized");
}

END_NAMESPACE_GMGRAPHICS;
