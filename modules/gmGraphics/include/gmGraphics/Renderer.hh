
#ifndef GRAMODS_GRAPHICS_RENDERER
#define GRAMODS_GRAPHICS_RENDERER

#include <gmGraphics/config.hh>

#include <gmGraphics/Node.hh>
#include <gmGraphics/Camera.hh>

#include <set>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   The base of graphics Renderer implementations.
 */
class Renderer : public gmGraphics::Node {

public:

  /**
     Performs rendering of 3D objects in the scene.

     It is assumed that depth test and blending is enabled upon
     calling this method.
  */
  virtual void render(const Camera &camera, const Eigen::Affine3f &Mm) = 0;

  /**
     Extracts the currently optimal near and far plane distances. This
     is typically used by a View if there are multiple renderers that
     need to be rendered with the same near and far planes for correct
     depth testing.
  */
  virtual void getNearFar(const Camera &camera,
                          const Eigen::Affine3f &Mm,
                          float &near,
                          float &far) = 0;

  /**
     Add an eye to render to. If no eye is specified, then all eyes
     should be rendered.
  */
  void addEye(Eye e);

  GM_OFI_DECLARE;

protected:
  /**
     Eyes to render. Sub classes should render consider a camera only
     if its eye is in this set or if the set is empty.
  */
  std::set<Eye> eyes;
};

END_NAMESPACE_GMGRAPHICS;

#endif
