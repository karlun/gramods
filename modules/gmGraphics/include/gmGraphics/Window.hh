
#ifndef GRAMODS_GRAPHICS_WINDOW
#define GRAMODS_GRAPHICS_WINDOW

#include <gmGraphics/config.hh>
#include <gmGraphics/Renderer.hh>

#include <gmTypes/all.hh>

#include <gmCore/Object.hh>
#include <gmCore/OFactory.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

class View;

/**
   The base of graphics Window implementations.
*/
class Window
  : public gmCore::Object {

public:

  Window();
  virtual ~Window() {}

  /**
     Asks the Window to call its views for rendering.
   */
  void renderFullPipeline();

  /**
     Adds a renderer to the window.
  */
  void addRenderer(std::shared_ptr<Renderer> renderer) {
    renderers_to_setup.push_back(renderer);
  }

  /**
     Adds a view to the window. A window without views will render
     nothing - it is the tiles that provide the graphics. If multiple
     views are added, then these will be rendered over each other.
  */
  void addView(std::shared_ptr<View> view);

  /**
     Asks the Window to make its GL context current. This is called
     automatically by the renderFullPipeline method.
  */
  virtual void makeGLContextCurrent() {}

  /**
     Asks the Window to close itself. This should be overloaded by sub
     classes to support run-time changes.
  */
  virtual void close() {}

  /**
     Activates or deactivates fullscreen mode. This should be
     overloaded by sub classes to also support run-time changes.
  */
  virtual void setFullscreen(bool on) { fullscreen = on; }

  /**
     Sets the size of the window. This should be overloaded by sub
     classes to also support run-time changes.
  */
  virtual void setSize(gmTypes::size2 s) { size = s; }

  /**
     Sets the title of the windows. This should be overloaded by sub
     classes to also support run-time changes.
  */
  virtual void setTitle(std::string t) { title = t; }

  /**
     Triggers the windows to process its incoming events. This must be
     called at even intervals for the window to behave properly.
  */
  virtual void processEvents() {}

  /**
     Returns true as long as the window is open. This does not
     necessarily mean that the window is visible and it may even be
     minimized.
  */
  virtual bool isOpen() { return false; }

  /**
     Finalizes the rendering and swaps the render buffers to show the
     newly rendered material.
  */
  virtual void swap() {}

  GM_OFI_DECLARE;

protected:

  std::vector<std::shared_ptr<Renderer>> renderers_to_setup;
  std::vector<std::shared_ptr<Renderer>> renderers;

  bool fullscreen;
  std::string title;
  gmTypes::size2 size;
  
};

END_NAMESPACE_GMGRAPHICS;

#endif
