
#ifndef GRAMODS_GRAPHICS_CUBESCENERENDERER
#define GRAMODS_GRAPHICS_CUBESCENERENDERER

#include <gmGraphics/Renderer.hh>

#include <gmTypes/eigen.hh>
#include <gmCore/OFactory.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   A renderer that draws a set of cubes, for demonstration and testing
   purposes.
*/
class CubeSceneRenderer
  : public Renderer {

public:

  CubeSceneRenderer();

  /**
     Performs rendering of 3D objects in the scene.
  */
  void render(Camera camera, float near = -1, float far = -1);

  /**
     Extracts the currently optimal near and far plane distances. This
     is typically used by a View if there are multiple renderers that
     need to be rendered with the same near and far planes for correct
     depth testing.
  */
  void getNearFar(Camera camera, float &near, float &far);

  /**
     Sets the size of each cube to be drawn.

     \b XML-attribute: \c cubeSize
  */
  void setCubeSize(float d);

  /**
     Sets the size of the set of cubes.

     \b XML-attribute: \c cubeSetSize
  */
  void setCubeSetSize(float d);

  /**
     Sets the position of the cube set.

     \b XML-attribute: \c position
  */
  void setPosition(Eigen::Vector3f c);

  /**
     Set animation on or off.

     \b XML-attribute: \c animate
  */
  void setAnimate(bool on);

  GM_OFI_DECLARE;

private:

  struct Impl;
  std::unique_ptr<Impl> _impl;

};

END_NAMESPACE_GMGRAPHICS;

#endif
