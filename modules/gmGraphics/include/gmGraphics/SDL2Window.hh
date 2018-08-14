
#ifndef GRAMODS_GRAPHICS_SDL2WINDOW
#define GRAMODS_GRAPHICS_SDL2WINDOW

#include <gmGraphics/Window.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   The base of graphics Window implementations.
 */
class SDL2Window
  : public Window {

public:

  /**
     Asks the Window to make its GL context current. This is called
     automatically by the renderFullPipeline method.
  */
  void makeGLContextCurrent();

  /**
     Called to initialize the Window. Sub classes to override this to
     make use of set parameter data. This should be called once only!
  */
  void initialize();

  GM_OFI_DECLARE(SDL2Window);

};

END_NAMESPACE_GMGRAPHICS;

#endif
