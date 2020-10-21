
#ifndef GRAMODS_GRAPHICS_CUBESCENERENDERER
#define GRAMODS_GRAPHICS_CUBESCENERENDERER

#include <gmGraphics/Renderer.hh>

#include <gmCore/eigen.hh>
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

     \gmXmlTag{gmGraphics,CubeSceneRenderer,cubeSize}
  */
  void setCubeSize(float d);

  /**
     Sets the size of the set of cubes.

     \gmXmlTag{gmGraphics,CubeSceneRenderer,cubeSetSize}
  */
  void setCubeSetSize(float d);

  /**
     Sets the position of the cube set.

     \gmXmlTag{gmGraphics,CubeSceneRenderer,position}

     \sa gramods::operator>>(std::istream &, Eigen::Vector3f &)
  */
  void setPosition(Eigen::Vector3f c);

  /**
     Set animation on or off.

     \gmXmlTag{gmGraphics,CubeSceneRenderer,animate}
  */
  void setAnimate(bool on);

  GM_OFI_DECLARE;

private:

  struct Impl;
  std::unique_ptr<Impl> _impl;

};

END_NAMESPACE_GMGRAPHICS;

#endif
