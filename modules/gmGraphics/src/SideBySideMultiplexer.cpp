
#include <gmGraphics/SideBySideMultiplexer.hh>

#include <GL/glew.h>
#include <GL/gl.h>

#define PATTERN_HORIZONTAL 0
#define PATTERN_VERTICAL 1

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE(SideBySideMultiplexer);
GM_OFI_PARAM(SideBySideMultiplexer, pattern, int, SideBySideMultiplexer::setPattern);

struct SideBySideMultiplexer::Impl {
  GLint viewport[4] = { 0, 0, 0, 0 };
  int pattern;
};

SideBySideMultiplexer::SideBySideMultiplexer()
  : _impl(std::make_unique<SideBySideMultiplexer::Impl>()) {}

void SideBySideMultiplexer::prepare() {
  glGetIntegerv(GL_VIEWPORT, _impl->viewport);
}

void SideBySideMultiplexer::setupRendering(Eye eye) {
  
  switch(eye) {

  case Eye::LEFT:

    if (_impl->pattern == 0)
      glViewport(_impl->viewport[0], _impl->viewport[1],
                 _impl->viewport[2] / 2, _impl->viewport[3]);
    else
      glViewport(_impl->viewport[0], _impl->viewport[1] + _impl->viewport[3]/2,
                 _impl->viewport[2], _impl->viewport[3] / 2);

    break;

  case Eye::RIGHT:

    if (_impl->pattern == 0)
      glViewport(_impl->viewport[0] + _impl->viewport[2] / 2, _impl->viewport[1],
                 _impl->viewport[2] / 2, _impl->viewport[3]);
    else
      glViewport(_impl->viewport[0], _impl->viewport[1],
                 _impl->viewport[2], _impl->viewport[3] / 2);

    break;
  }
}

void SideBySideMultiplexer::finalize() {
  glViewport(_impl->viewport[0], _impl->viewport[1], _impl->viewport[2], _impl->viewport[3]);
}

void SideBySideMultiplexer::setPattern(int p) {
  if (p < 0 || 2 < p)
    throw std::invalid_argument("invalid interlace pattern");
  _impl->pattern = p;
}

END_NAMESPACE_GMGRAPHICS;
