
#ifndef GRAMODS_GRAPHICS_SDLWINDOW
#define GRAMODS_GRAPHICS_SDLWINDOW

#include <gmCore/SdlContext.hh>

#ifdef gramods_ENABLE_SDL2

#include <gmGraphics/Window.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   An SDL window implementation.
*/
class SdlWindow
  : public Window {

public:

  SdlWindow();
  ~SdlWindow();

  void initialize() override;

  /**
     Asks the Window to make its GL context current. This is called
     automatically by the renderFullPipeline method.
  */
  void makeGLContextCurrent() override;

  /**
     Returns the size of the drawable canvas of this window, in
     pixels.
  */
  gmCore::size2 getSize();

  /**
     Close window.
  */
  void close() override;

  /**
     Sets if quad buffers should be requested for the GL context.

     \gmXmlTag{gmGraphics,SdlWindow,useQuadBuffers}
  */
  void setUseQuadBuffers(bool on) { gl_use_quad_buffers = on; }

  /**
     Sets the major version to be requested for the GL context in this
     Window.

     \gmXmlTag{gmGraphics,SdlWindow,GLMajor}
  */
  void setGLMajor(int v) { gl_major = v; }

  /**
     Sets the minor version to be requested for the GL context in this
     Window.

     \gmXmlTag{gmGraphics,SdlWindow,GLMinor}
  */
  void setGLMinor(int v) { gl_minor = v; }

  /**
     Sets the profile to be requested for the GL context in this
     Window. Valid values are CORE, ES and COMPATIBILITY.

     \gmXmlTag{gmGraphics,SdlWindow,GLProfile}
  */
  void setGLProfile(std::string s) { gl_profile = s; }

  /**
     The SDL context. Providing the context pointer to the SDL window
     is not strictly necessary, but putting the SDL context here
     ensures that it is not destroyed until the SdlWindow has been
     destroyed.

     \gmXmlTag{gmGraphics,SdlWindow,context}
  */
  void setContext(std::shared_ptr<gmCore::SdlContext> ctx) { context = ctx; }

  /**
     Triggers the windows to process its incoming events. This must be
     called at even intervals for the window to behave properly.
  */
  void processEvents() override;

  /**
     Returns true as long as the window is open. This does not
     necessarily mean that the window is visible.
  */
  bool isOpen() override;

  /**
     Finalizes the rendering and swaps the render buffers to show the
     newly rendered material.
  */
  void swap() override;

  /**
     Propagates the specified visitor.

     @see Object::Visitor
  */
  void traverse(Visitor *visitor) override;

  GM_OFI_DECLARE;

private:

  bool processEvent(SDL_Event& event);

  bool alive;
  SDL_Window* window;
  SDL_GLContext gl_context;
  static std::map<unsigned int, std::weak_ptr<SdlWindow>> sdl_windows;

  bool gl_use_quad_buffers = false;
  int gl_major = -1;
  int gl_minor = -1;
  std::string gl_profile;

  std::shared_ptr<gmCore::SdlContext> context;
};

END_NAMESPACE_GMGRAPHICS;

#endif
#endif
