
#ifndef GRAMODS_GRAPHICS_OSGRENDERER
#define GRAMODS_GRAPHICS_OSGRENDERER

#include <gmGraphics/Renderer.hh>

#ifdef gramods_ENABLE_OpenSceneGraph

namespace osg {
  class Node;
}

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   A renderer that renders an OpenSceneGraph scenegraph.
*/
class OsgRenderer
  : public Renderer {

public:

  OsgRenderer();
  ~OsgRenderer();

  /**
     Calls the scenegraph root for rendering.
  */
  void render(Camera camera, float near = -1, float far = -1);

  /**
     Extracts the currently optimal near and far plane distances.
  */
  void getNearFar(Camera camera, float &near, float &far);

  /**
     Sets the scenegraph.
  */
  void setSceneData(osg::Node *node);

private:
  struct Impl;
  std::unique_ptr<Impl> _impl;
};

END_NAMESPACE_GMGRAPHICS;

#endif
#endif
