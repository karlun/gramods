
#ifndef GRAMODS_GRAPHICS_WINDOW
#define GRAMODS_GRAPHICS_WINDOW

#include <gmGraphics/config.hh>
#include <gmGraphics/Camera.hh>

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

  /**
     Asks the Window to call its views for rendering.
   */
  void renderFullPipeline(Camera::RenderFunction *func = 0);

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
     Sets the window size to the specified values.
  */
  //virtual void setWindowSize(glm::ivec2 size) = 0;

  /**
     Activates or deactivates fullscreen mode. This should be
     overloaded by sub classes to support run-time changes.
  */
  virtual void setFullscreen(bool on) { fullscreen = on; }

  /**
     Sets the size of the window. This should be overloaded by sub
     classes to support run-time changes.
  */
  virtual void setSize(gmTypes::size2 s) { size = s; }

  /**
     Sets the title of the windows. This should be overloaded by sub
     classes to support run-time changes.
  */
  virtual void setTitle(std::string t) { title = t; }

  GM_OFI_DECLARE(Window);

protected:

  bool fullscreen;
  std::string title;
  gmTypes::size2 size;
  
};

END_NAMESPACE_GMGRAPHICS;

#endif
