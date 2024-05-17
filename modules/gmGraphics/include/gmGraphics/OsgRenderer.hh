
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
     Sets the current frame number. This is propagated to the OSG
     viewer to be used for example in animation updates. This is
     automatically incremented if not set.
  */
  void setFrameNumber(unsigned int n);

  /**
     Calls the scenegraph root for rendering.
  */
  void render(const Camera &camera, const Eigen::Affine3f &Mm) override;

  /**
     Extracts the currently optimal near and far plane distances.
  */
  void getNearFar(const Camera &camera,
                  const Eigen::Affine3f &Mm,
                  float &near,
                  float &far) override;

  /**
     Sets the scenegraph.
  */
  void setSceneData(osg::Node *node);

  /**
     Called to make the object up-to-date.
  */
  void update(clock::time_point time, size_t frame) override;

private:
  struct Impl;
  std::unique_ptr<Impl> _impl;
};

END_NAMESPACE_GMGRAPHICS;

#endif
#endif
