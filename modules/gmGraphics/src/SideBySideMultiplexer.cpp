
#include <gmGraphics/SideBySideMultiplexer.hh>

#include <GL/glew.h>
#include <GL/gl.h>

#define PATTERN_HORIZONTAL    0
#define PATTERN_VERTICAL      1
#define PATTERN_FRAME_PACKING 2

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE(SideBySideMultiplexer);
GM_OFI_PARAM2(SideBySideMultiplexer, pattern, int, setPattern);

struct SideBySideMultiplexer::Impl {
  GLint viewport[4] = { 0, 0, 0, 0 };
  int pattern;
};

SideBySideMultiplexer::SideBySideMultiplexer()
  : _impl(std::make_unique<SideBySideMultiplexer::Impl>()) {}

void SideBySideMultiplexer::prepare() {
  glGetIntegerv(GL_VIEWPORT, _impl->viewport);
}

void SideBySideMultiplexer::setupRendering(size_t eye) {

  switch(_impl->pattern) {

  case PATTERN_HORIZONTAL:

    switch(eye) {

    case 0:
      glViewport(_impl->viewport[0], _impl->viewport[1],
                 _impl->viewport[2] / 2, _impl->viewport[3]);
      break;

    case 1:
      glViewport(_impl->viewport[0] + _impl->viewport[2] / 2, _impl->viewport[1],
                 _impl->viewport[2] / 2, _impl->viewport[3]);
      break;

    default:
      throw gmCore::InvalidArgument("cannot render eye index higher than 1");
    }

    break;

  case PATTERN_VERTICAL:

    switch(eye) {

    case 0:
      glViewport(_impl->viewport[0], _impl->viewport[1] + _impl->viewport[3] / 2,
                 _impl->viewport[2], _impl->viewport[3] / 2);
      break;

    case 1:
      glViewport(_impl->viewport[0], _impl->viewport[1],
                 _impl->viewport[2], _impl->viewport[3] / 2);
      break;

    default:
      throw gmCore::InvalidArgument("cannot render eye index higher than 1");
    }

    break;

  case PATTERN_FRAME_PACKING:

    switch(eye) {

    case 0:
      glViewport(_impl->viewport[0], _impl->viewport[1] + (_impl->viewport[3] - 45) / 2 + 45,
                 _impl->viewport[2], (_impl->viewport[3] - 45) / 2);
      break;

    case 1:
      glViewport(_impl->viewport[0], _impl->viewport[1],
                 _impl->viewport[2], (_impl->viewport[3] - 45) / 2);
      break;

    default:
      throw gmCore::InvalidArgument("cannot render eye index higher than 1");
    }

    break;

  default:
    assert(0);
  }
}

void SideBySideMultiplexer::finalize() {
  glViewport(_impl->viewport[0], _impl->viewport[1], _impl->viewport[2], _impl->viewport[3]);
}

void SideBySideMultiplexer::setPattern(int p) {
  if (p < 0 || 2 < p)
    throw gmCore::InvalidArgument("invalid pattern");
  _impl->pattern = p;
}

END_NAMESPACE_GMGRAPHICS;
