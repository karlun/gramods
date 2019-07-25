
#include <gmGraphics/QuadBufferMultiplexer.hh>

#include <GL/glew.h>
#include <GL/gl.h>

BEGIN_NAMESPACE_GMGRAPHICS;

struct QuadBufferMultiplexer::Impl {
  GLint buffer_id;
};

QuadBufferMultiplexer::QuadBufferMultiplexer()
  : _impl(std::make_unique<QuadBufferMultiplexer::Impl>()) {}

void QuadBufferMultiplexer::prepare() {
  glGetIntegerv(GL_DRAW_BUFFER, &_impl->buffer_id);
}

void QuadBufferMultiplexer::setupRendering(size_t eye) {

  switch(eye) {

  case 0:
    glDrawBuffer(GL_BACK_LEFT);
    break;

  case 1:
    glDrawBuffer(GL_BACK_RIGHT);
    break;

  default:
    throw std::invalid_argument("cannot render eye index higher than 1");
  }
}

void QuadBufferMultiplexer::finalize() {
  glDrawBuffer(_impl->buffer_id);
}

END_NAMESPACE_GMGRAPHICS;
