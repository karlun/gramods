
#ifndef GRAMODS_CORE_FREEIMAGE
#define GRAMODS_CORE_FREEIMAGE

#include <gmCore/config.hh>

#ifdef gramods_ENABLE_FreeImage

#include <memory>

BEGIN_NAMESPACE_GMCORE;

/**
   Initializer for FreeImage. FreeImage will be in initialized state
   as long as a shared pointer is held to it.
*/
class FreeImage {

public:

  static std::shared_ptr<FreeImage> get();

  ~FreeImage();

private:

  FreeImage();

};

END_NAMESPACE_GMCORE;

#endif
#endif
