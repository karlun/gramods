
#ifndef GRAMODS_GRAPHICS_VIEWBASE
#define GRAMODS_GRAPHICS_VIEWBASE

#include <gmGraphics/config.hh>
#include <gmGraphics/Renderer.hh>
#include <gmGraphics/Viewpoint.hh>

#include <gmCore/Object.hh>
#include <gmCore/OFactory.hh>

#include <GL/glew.h>
#include <GL/gl.h>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   The common base for nodes taking renderers for dispatching
   rendering jobs.
*/
class ViewBase
  : public gmCore::Object {

public:

  /**
     Package of settings affecting the rendering.
  */
  struct ViewSettings {

    ViewSettings() = delete;

    ViewSettings(size_t frame_number)
      : frame_number(frame_number) {}

    /// Creates settings associated with the specified viewpoint.
    ViewSettings(size_t frame_number, std::shared_ptr<Viewpoint> vp)
      : frame_number(frame_number), viewpoints({vp}) {}

    /// Creates settings associated with the specified viewpoints.
    ViewSettings(size_t frame_number,
                 std::vector<std::shared_ptr<Viewpoint>> vps)
      : frame_number(frame_number), viewpoints(vps) {}

    /// The frame currently being rendered. This value is increased by
    /// one for every time the rendering loop is executed and can thus
    /// be used to track e.g. caching.
    size_t frame_number;

    /// The renderers to render in the view.
    Renderer::list renderers;

    /// The viewpoint currently being rendered.
    std::vector<std::shared_ptr<Viewpoint>> viewpoints;

    /// The preferred pixel format for buffers.
    GLenum pixel_format = GL_RGBA8;
  };

  /**
     Dispatches specified renders, both method argument and class
     members. The default implementation calls the renderers but with
     a default Camera, and should be overloaded by sub classes to
     implement their respective effects.
  */
  virtual void renderFullPipeline(ViewSettings settings);

  /**
     Adds a renderer to the view.
  */
  void addRenderer(std::shared_ptr<Renderer> renderer) {
    renderers.push_back(renderer);
  }

  /**
     Removes the specified renderer from the view. Does nothing if the
     specified renderer has not been added.
  */
  void removeRenderer(std::shared_ptr<Renderer> renderer) {
    renderers.erase(std::remove(renderers.begin(),
                                renderers.end(), renderer),
                    renderers.end());
  }

  /**
     Removes all renderers on this dispatcher and, if recursive is set
     to true, also renderers added to sub dispatchers.

     Sub classes that have sub dispatchers should override this method
     to, when requested, call the clearRenderers method of those sub
     dispatchers.
  */
  virtual void clearRenderers(bool recursive = false);

  /**
     Sets the viewpoint to use in the views rendered by this
     ViewBase.
  */
  void setViewpoint(std::shared_ptr<Viewpoint> viewpoint) {
    viewpoints = {viewpoint};
  }

  /**
     Adds a viewpoint to use in the views rendered by this
     ViewBase.

     \gmXmlTag{gmGraphics,ViewBase,viewpoint}
  */
  void addViewpoint(std::shared_ptr<Viewpoint> viewpoint) {
    viewpoints.push_back(viewpoint);
  }

  GM_OFI_DECLARE;

protected:

  typedef std::vector<std::shared_ptr<Renderer>> renderer_list;

  /**
     Adds the dispatcher's local renderers and viewpoint, if set. Call
     this from overloaded renderFullPipeline.
  */
  void populateViewSettings(ViewSettings &settings);

  Renderer::list renderers;
  std::vector<std::shared_ptr<Viewpoint>> viewpoints;

};

END_NAMESPACE_GMGRAPHICS;

#endif
