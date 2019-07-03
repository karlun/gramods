
#ifndef GRAMODS_GRAPHICS_SPHERERENDERER
#define GRAMODS_GRAPHICS_SPHERERENDERER

#include <gmGraphics/Renderer.hh>
#include <gmGraphics/Texture.hh>
#include <gmGraphics/CoordinatesMapper.hh>

#include <gmTypes/eigen.hh>
#include <gmCore/OFactory.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   A renderer that draws single sphere, for demonstration and testing
   purposes.
*/
class SphereRenderer
  : public Renderer {

public:

  SphereRenderer();

  /**
     Performs rendering of 3D objects in the scene.
  */
  void render(Camera camera);

  /**
     Set the radius of the sphere.

     \b XML-attribute: \c radius
  */
  void setRadius(float r);

  /**
     Set the position of the sphere.

     \b XML-attribute: \c center
  */
  void setCenter(Eigen::Vector3f c);

  /**
     Sets the Texture to apply on the sphere.

     \b XML-key: \c texture
  */
  void setTexture(std::shared_ptr<Texture> tex);

  /**
     Sets the coordinates mapper that should be used to map the 2D
     texture onto the 3D sphere.
  */
  void setCoordinatesMapper(std::shared_ptr<CoordinatesMapper> mapper);

  GM_OFI_DECLARE;

private:

  class Impl;
  std::unique_ptr<Impl> _impl;

};

END_NAMESPACE_GMGRAPHICS;

#endif
