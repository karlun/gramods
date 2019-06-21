
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
  void render(Camera camera);

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

     \b XML-attribute: \c cubeSetCenter
  */
  void setCubeSetCenter(Eigen::Vector3f c);

  GM_OFI_DECLARE;

private:

  class Impl;
  std::unique_ptr<Impl> _impl;

};

END_NAMESPACE_GMGRAPHICS;

#endif
