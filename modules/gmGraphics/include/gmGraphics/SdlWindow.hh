
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

  void initialize();

  /**
     Asks the Window to make its GL context current. This is called
     automatically by the renderFullPipeline method.
  */
  void makeGLContextCurrent();

  /**
     The SDL context. Providing the context pointer to the SDL window
     is not strictly necessary, but putting the SDL context here
     ensures that it is not destroyed until the SdlWindow has been
     destroyed.
  */
  void setContext(std::shared_ptr<gmCore::SdlContext> ctx) { context = ctx; }

  /**
     Triggers the windows to process its incoming events. This must be
     called at even intervals for the window to behave properly.
  */
  void processEvents();

  /**
     Returns true as long as the window is open. This does not
     necessarily mean that the window is visible.
  */
  bool isOpen();

  /**
     Finalizes the rendering and swaps the render buffers to show the
     newly rendered material.
  */
  void swap();

  GM_OFI_DECLARE;

private:

  bool handleEvent(SDL_Event& event);

  bool alive;
  SDL_Window* window;
  SDL_GLContext gl_context;

  std::shared_ptr<gmCore::SdlContext> context;
};

END_NAMESPACE_GMGRAPHICS;

#endif
