
#include <gmGraphics/QuadBufferMultiplexer.hh>

#include <gmGraphics/OffscreenRenderTargets.hh>
#include <gmGraphics/RasterProcessor.hh>

#include <gmCore/Console.hh>
#include <gmCore/RunOnce.hh>

#include <GL/glew.h>
#include <GL/gl.h>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE(QuadBufferMultiplexer);

namespace {
const char *vertex_shader_code = R"lang=glsl(
#version 330 core

in vec2 a_vertex;
out vec2 v_uv;

void main() {
  v_uv = a_vertex * 0.5 - 0.5;
  gl_Position = vec4(a_vertex, 0.0, 1.0);
}
)lang=glsl";

const char *fragment_shader_code = R"lang=glsl(
#version 330 core

uniform sampler2D tex;

in vec2 v_uv;
out vec4 fragColor;

void main() {
  fragColor = texture(tex, v_uv);
}
)lang=glsl";
}

struct QuadBufferMultiplexer::Impl {
  void prepare();
  void setupRendering(size_t eye);
  void finalize();

  OffscreenRenderTargets render_targets;
  RasterProcessor raster_processor;

  bool is_setup = false;
  bool is_functional = false;

  GLint buffer_id;
  GLint uniform_tex = 0;
};

QuadBufferMultiplexer::QuadBufferMultiplexer()
  : _impl(std::make_unique<QuadBufferMultiplexer::Impl>()) {}

void QuadBufferMultiplexer::prepare() { _impl->prepare(); }

void QuadBufferMultiplexer::Impl::prepare() {
  if (!is_setup) {
    is_setup = true;
    raster_processor.setVertexCode(vertex_shader_code);
    raster_processor.setFragmentCode(fragment_shader_code);
    if (render_targets.init(2) && raster_processor.init()) is_functional = true;
  }

  if (!is_functional) {
    GM_RUNONCE(
        GM_ERR("QuadBufferMultiplexer", "Dysfunctional internal GL workings."));
    return;
  }

  glGetIntegerv(GL_DRAW_BUFFER, &buffer_id);
  render_targets.push();
}

void QuadBufferMultiplexer::setupRendering(size_t eye) {
  _impl->setupRendering(eye);
}

void QuadBufferMultiplexer::Impl::setupRendering(size_t eye) {
  if (!is_functional) return;

  switch(eye) {

  case 0: render_targets.bind(0, 0, 0); break;

  case 1: render_targets.bind(0, 0, 1); break;

  default:
    throw gmCore::InvalidArgument("cannot render eye index higher than 1");
  }
}

void QuadBufferMultiplexer::finalize() { _impl->finalize(); }

#define LOC(VAR, NAME)                                                         \
  (VAR > 0 ? VAR : (VAR = glGetUniformLocation(program_id, NAME)))

void QuadBufferMultiplexer::Impl::finalize() {
  if (!is_functional) return;

  render_targets.pop();
  GM_DBG2(
      "QuadBufferMultiplexer",
      "Render left and right eye buffer to left and right back buffers, respectively");

  glDisable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glActiveTexture(GL_TEXTURE0);

  GLuint program_id = raster_processor.getProgramId();
  glUseProgram(program_id);
  glUniform1i(LOC(uniform_tex, "tex"), 0);

  glBindTexture(GL_TEXTURE_2D, render_targets.getTexId(0));
  glDrawBuffer(GL_BACK_LEFT);
  raster_processor.run();

  glBindTexture(GL_TEXTURE_2D, render_targets.getTexId(1));
  glDrawBuffer(GL_BACK_RIGHT);
  raster_processor.run();

  glBindTexture(GL_TEXTURE_2D, 0);

  glDrawBuffer(buffer_id);
}

END_NAMESPACE_GMGRAPHICS;
