
#ifndef GRAMODS_GRAPHICS_WINDOW
#define GRAMODS_GRAPHICS_WINDOW

// Required before gmCore/OFactory.hh for some compilers
#include <gmCore/io_float.hh>
#include <gmCore/io_int.hh>
#include <gmCore/io_size.hh>

#include <gmGraphics/ViewBase.hh>

#include <limits>

BEGIN_NAMESPACE_GMGRAPHICS;

class View;

/**
   The base of graphics Window implementations.
*/
class Window : public ViewBase {

public:

  struct event {
    virtual ~event() {}
  };

  struct key_event : event {
    bool up;
    int key;
    key_event(bool up, int key) : up(up), key(key) {}
    virtual ~key_event() {}
  };

  Window();
  virtual ~Window() {}

  /**
     Dispatches specified renders, both method argument and class
     members, to the views in this window.
  */
  void renderFullPipeline(ViewSettings settings) override;

  /**
     Propagates the specified visitor.

     @see Object::Visitor
  */
  void traverse(Visitor *visitor) override;

  /**
     Adds a view to the window. A window without views will render
     nothing - it is the tiles that provide the graphics. If multiple
     views are added, then these will be rendered over each other.

     \gmXmlTag{gmGraphics,Window,view}
  */
  void addView(std::shared_ptr<View> view) {
    if (!view) throw gmCore::InvalidArgument("null not allowed");
    views.push_back(view);
  }

  /**
     Asks the Window to make its GL context current. This is called
     automatically by the renderFullPipeline method.
  */
  virtual void makeGLContextCurrent() = 0;

  /**
     Asks the Window to close itself. This should be overloaded by sub
     classes to support run-time changes.
  */
  virtual void close() = 0;

  /**
     Activates or deactivates fullscreen mode. This should be
     overloaded by sub classes to also support run-time changes.

     \gmXmlTag{gmGraphics,Window,fullscreen}
  */
  virtual void setFullscreen(bool on) { fullscreen = on; }

  /**
     Set on which display the window should be shown.

     \gmXmlTag{gmGraphics,Window,display}
  */
  virtual void setDisplay(size_t N) { display = N; }

  /**
     Sets the size of the drawable canvas of this window, in
     pixels. This should be overloaded by sub classes to also support
     run-time changes.

     \gmXmlTag{gmGraphics,Window,size}
  */
  virtual void setSize(gmCore::size2 s) { size = s; }

  /**
     Sets the position of the window.

     \gmXmlTag{gmGraphics,Window,position}
  */
  virtual void setPosition(gmCore::int2 p) { position = p; }

  /**
     Returns the size of the drawable canvas of this window, in
     pixels. This should be overloaded by sub classes to also support
     run-time changes.
  */
  virtual gmCore::size2 getSize() { return size; }

  /**
     Sets the title of the windows. This should be overloaded by sub
     classes to also support run-time changes.

     \gmXmlTag{gmGraphics,Window,title}
  */
  virtual void setTitle(std::string t) { title = t; }

  /**
     Set the background color of the window.

     \gmXmlTag{gmGraphics,Window,backgroundColor}
  */
  virtual void setBackgroundColor(gmCore::float4 c) {
    background_color = c;
  }

  /**
     Triggers the windows to process its incoming events. This must be
     called at even intervals for the window to behave properly.
  */
  virtual void processEvents() = 0;

  /**
     Internal events handler, called from processEvents(). This will
     in turn call any registered event handler.
  */
  virtual bool handleEvent(event *);

  /**
     Registers an event handler associated with the specified tag. The
     tag can be a pointer to any object and is only used for calls to
     removeEventHandler.
  */
  void addEventHandler(std::function<bool(const event*)> fun, void *tag);

  /**
     Removes the event handler associnated with the specified tag.
  */
  void removeEventHandler(void *tag);

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
  virtual void swap() = 0;

  /**
     Calls glFinish on the GL context. The only reason to call this
     method is if the application needs to wait for vertical scan
     synchronization (v-sync), for example for timing reasons.
  */
  virtual void sync();

  GM_OFI_DECLARE;

protected:

  std::vector<std::shared_ptr<View>> views;

  bool fullscreen = false;
  size_t display = 0;
  std::string title = "untitled gramods window";
  gmCore::size2 size = {640, 480};
  gmCore::int2 position = { (std::numeric_limits<int>::max)(),
                             (std::numeric_limits<int>::max)() };
  gmCore::float4 background_color = {0.f, 0.f, 0.f, 0.f};

  std::unordered_map<void*, std::function<bool(const event*)>> event_handlers;
};

END_NAMESPACE_GMGRAPHICS;

#endif
