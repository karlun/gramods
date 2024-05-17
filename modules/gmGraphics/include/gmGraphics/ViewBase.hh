
#ifndef GRAMODS_GRAPHICS_VIEWBASE
#define GRAMODS_GRAPHICS_VIEWBASE

#include <gmGraphics/config.hh>

#include <gmGraphics/Node.hh>
#include <gmGraphics/Camera.hh>
#include <gmGraphics/Viewpoint.hh>

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

    /**
       Sends render visitor to the nodes.
    */
    void renderNodes(Camera camera);

    /// The frame currently being rendered. This value is increased by
    /// one for every time the rendering loop is executed and can thus
    /// be used to track e.g. caching.
    size_t frame_number;

    /// The renderers to render in the view.
    Node::list nodes;

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
     Adds a scenegraph to the view.
  */
  void addNode(std::shared_ptr<Node> node) {
    nodes.push_back(node);
  }

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

  /**
     Propagates the specified visitor.

     @see Object::Visitor
  */
  void traverse(Visitor *visitor) override;

  GM_OFI_DECLARE;

protected:
  /**
     Adds the dispatcher's local renderers and viewpoint, if set. Call
     this from overloaded renderFullPipeline.
  */
  void populateViewSettings(ViewSettings &settings);

  Node::list nodes;
  std::vector<std::shared_ptr<Viewpoint>> viewpoints;
};

END_NAMESPACE_GMGRAPHICS;

#endif
