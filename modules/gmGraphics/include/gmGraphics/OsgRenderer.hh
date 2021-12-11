
#ifndef GRAMODS_GRAPHICS_OSGRENDERER
#define GRAMODS_GRAPHICS_OSGRENDERER

#include <gmGraphics/Renderer.hh>

#ifdef gramods_ENABLE_OpenSceneGraph

#include <gmCore/Updateable.hh>

namespace osg {
  class Node;
}

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   A renderer that renders an OpenSceneGraph scenegraph.
*/
class OsgRenderer
  : public Renderer,
    public gmCore::Updateable {

public:

  OsgRenderer();
  ~OsgRenderer();

  /**
     Calls the scenegraph root for rendering.
  */
  void render(Camera camera, float near = -1, float far = -1) override;

  /**
     Extracts the currently optimal near and far plane distances.
  */
  void getNearFar(Camera camera, float &near, float &far) override;

  /**
     Sets the scenegraph.
  */
  void setSceneData(osg::Node *node);

  /**
     Called to make the object up-to-date.
  */
  void update(clock::time_point t) override;

private:
  struct Impl;
  std::unique_ptr<Impl> _impl;
};

END_NAMESPACE_GMGRAPHICS;

#endif
#endif
