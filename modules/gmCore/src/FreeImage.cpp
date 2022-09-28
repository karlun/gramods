
#include <gmCore/FreeImage.hh>

#ifdef gramods_ENABLE_FreeImage

#include <gmCore/Console.hh>

#include <FreeImage.h>

BEGIN_NAMESPACE_GMCORE;

std::shared_ptr<FreeImage> FreeImage::get() {
  static std::weak_ptr<FreeImage> singleton;
  std::shared_ptr<FreeImage> locked = singleton.lock();
  if (!locked) {
    locked.reset(new FreeImage());
    singleton = locked;
  }
  return locked;
}

namespace {
  void freeimage_output(FREE_IMAGE_FORMAT, const char *msg) {
    GM_ERR("FreeImage", msg);
  }
}

FreeImage::FreeImage() {
  FreeImage_Initialise();
  FreeImage_SetOutputMessage(freeimage_output);
}

FreeImage::~FreeImage() {
  FreeImage_DeInitialise();
}

END_NAMESPACE_GMCORE;

#endif
