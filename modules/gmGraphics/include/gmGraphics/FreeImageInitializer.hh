
#ifndef GRAMODS_GRAPHICS_FREEIMAGEINITIALIZER
#define GRAMODS_GRAPHICS_FREEIMAGEINITIALIZER

#include <gmGraphics/config.hh>

#ifdef gramod_ENABLE_FreeImage

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   Singleton initializing the freeimage library and deinitializing it
   when noone is using it anymore.
*/
class FreeImageInitializer {

protected:

  /**
     Initializes the FreeImage library.
  */
  FreeImageInitializer();

  /**
     Deinitializes the FreeImage library.
  */
  ~FreeImageInitializer();

public:

  /**
     Returns the initializer instance. As long as this instance is
     alive the library is initialized.
  */
  static std::shared_ptr<FreeImageInitializer> get() {

    static std::weak_ptr<FreeImageInitializer> ptr;
    std::shared_ptr<FreeImageInitializer> lptr = ptr.lock();

    if (!lptr) {
      lptr = std::make_shared<FreeImageInitializer>();
      ptr = lptr;
    }

    return lptr;
  }
};

END_NAMESPACE_GMGRAPHICS;

#endif
#endif
