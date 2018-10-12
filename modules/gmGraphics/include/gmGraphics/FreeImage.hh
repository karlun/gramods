
#ifndef GRAMODS_GRAPHICS_FREEIMAGE
#define GRAMODS_GRAPHICS_FREEIMAGE

#include <gmGraphics/config.hh>

#ifdef gramods_ENABLE_FreeImage

#include <memory>

BEGIN_NAMESPACE_GMGRAPHICS;

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

END_NAMESPACE_GMGRAPHICS;

#endif
#endif
