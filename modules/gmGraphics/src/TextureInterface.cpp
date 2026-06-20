
#include <gmGraphics/TextureInterface.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

std::string TextureInterface::getRgbSwizzle(TextureColor c) {
  switch (c) {
  case GRAY: return "rrr";
  case RGB: return "rgb";
  case BGR: return "bgr";
  default: return "unknown";
  }
}

std::string TextureInterface::getRgbaSwizzle(TextureColor c) {
  switch (c) {
  case GRAY: return "rrra";
  case RGB: return "rgba";
  case BGR: return "bgra";
  default: return "unknown";
  }
}

END_NAMESPACE_GMGRAPHICS;
