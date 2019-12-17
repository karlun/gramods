
#ifndef GRAMODS_GRAPHICS_WINDOW
#define GRAMODS_GRAPHICS_WINDOW

#include <gmGraphics/RendererDispatcher.hh>

#include <gmTypes/size.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

class View;

/**
   The base of graphics Window implementations.
*/
class Window
  : public RendererDispatcher {

public:

  Window();
  virtual ~Window() {}

  /**
     Dispatches specified renders, both method argument and class
     members, to the views in this window.
  */
  void renderFullPipeline(ViewSettings settings);

  /**
     Adds a view to the window. A window without views will render
     nothing - it is the tiles that provide the graphics. If multiple
     views are added, then these will be rendered over each other.

     \b XML-key: \c view
  */
  void addView(std::shared_ptr<View> view) {
    views.push_back(view);
  }

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

     \b XML-attribute: \c fullscreen
  */
  virtual void setFullscreen(bool on) { fullscreen = on; }

  /**
     Sets the size of the drawable canvas of this window, in
     pixels. This should be overloaded by sub classes to also support
     run-time changes.

     \b XML-attribute: \c size
  */
  virtual void setSize(gmTypes::size2 s) { size = s; }

  /**
     Returns the size of the drawable canvas of this window, in
     pixels. This should be overloaded by sub classes to also support
     run-time changes.
  */
  virtual gmTypes::size2 getSize() { return size; }

  /**
     Sets the title of the windows. This should be overloaded by sub
     classes to also support run-time changes.

     \b XML-attribute: \c title
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
     Posts a swap buffers command to the underlying windowing system,
     to show the newly rendered material. Most drivers allow this
     command to return immediately, however subsequent calls to render
     to the same context may stall to wait for vertical scan
     synchronization (v-sync).
  */
  virtual void swap() {}

  /**
     Calls glFinish on the GL context. The only reason to call this
     method is if the application needs to wait for vertical scan
     synchronization (v-sync), for example for timing reasons.
  */
  virtual void sync() {}

  /**
     Removes all renderers and, if recursive is set to true, also
     renderers added to sub dispatchers.
  */
  virtual void clearRenderers(bool recursive = false);

  GM_OFI_DECLARE;

protected:

  std::vector<std::shared_ptr<View>> views;

  bool fullscreen;
  std::string title;
  gmTypes::size2 size;
  
};

END_NAMESPACE_GMGRAPHICS;

#endif
