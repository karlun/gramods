
#ifndef GRAMODS_GRAPHICS_SDLWINDOW
#define GRAMODS_GRAPHICS_SDLWINDOW

#include <gmGraphics/Window.hh>
#include <gmCore/SdlContext.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   An SDL window implementation.
*/
class SdlWindow
  : public Window {

public:

  SdlWindow();
  ~SdlWindow();

  /**
     Asks the Window to make its GL context current. This is called
     automatically by the renderFullPipeline method.
  */
  void makeGLContextCurrent();

  void initialize();

  /**
     The SDL context. Providing the context pointer to the SDL window
     is not strictly necessary, but putting the SDL context here
     ensures that it is not destroyed until the SdlWindow has been
     destroyed.
  */
  void setContext(std::shared_ptr<gmCore::SdlContext> ctx) { context = ctx; }

  GM_OFI_DECLARE;

private:

  SDL_Window* window;
  SDL_GLContext gl_context;

  std::shared_ptr<gmCore::SdlContext> context;
};

END_NAMESPACE_GMGRAPHICS;

#endif
