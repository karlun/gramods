
#ifndef GRAMODS_GRAPHICS_RENDERERDISPATCHER
#define GRAMODS_GRAPHICS_RENDERERDISPATCHER

#include <gmGraphics/config.hh>
#include <gmGraphics/Renderer.hh>
#include <gmGraphics/Viewpoint.hh>

#include <gmCore/Object.hh>
#include <gmCore/OFactory.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   The common base for nodes taking renderers for dispatching
   rendering jobs.
*/
class RendererDispatcher
  : public gmCore::Object {

public:

  /**
     Package of settings affecting the rendering.
  */
  struct ViewSettings {

    ViewSettings() {}
    ViewSettings(std::shared_ptr<Viewpoint> vp) : viewpoint(vp) {}

    std::vector<std::shared_ptr<Renderer>> renderers;
    std::shared_ptr<Viewpoint> viewpoint;
    Viewpoint::Eye eye_to_render;
  };

  /**
     Dispatches specified class members renders. This default
     implementation calls setup on the member renderers, but render
     nothing and should be overloaded by sub classes to implement
     their respective effects.
  */
  void renderFullPipeline() {
    renderFullPipeline(ViewSettings(viewpoint));
  }

  /**
     Dispatches specified renders, both method argument and class
     members. This default implementation calls setup on the member
     renderers, but render nothing and should be overloaded by sub
     classes to implement their respective effects.
  */
  virtual void renderFullPipeline(ViewSettings settings);

  /**
     Adds a renderer to the view.
  */
  void addRenderer(std::shared_ptr<Renderer> renderer) {
    renderers_to_setup.push_back(renderer);
  }

  /**
     Sets the viewpoint to use in the views rendered by this
     RendererDispatcher.
  */
  void setViewpoint(std::shared_ptr<Viewpoint> viewpoint) {
    this->viewpoint = viewpoint;
  }

  GM_OFI_DECLARE;

protected:

  std::vector<std::shared_ptr<Renderer>> renderers_to_setup;
  std::vector<std::shared_ptr<Renderer>> renderers;
  std::shared_ptr<Viewpoint> viewpoint;

};

END_NAMESPACE_GMGRAPHICS;

#endif
