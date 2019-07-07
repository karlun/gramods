
#ifndef GRAMODS_GRAPHICS_SPHERERENDERER
#define GRAMODS_GRAPHICS_SPHERERENDERER

#include <gmGraphics/Renderer.hh>
#include <gmGraphics/Texture.hh>
#include <gmGraphics/CoordinatesMapper.hh>

#include <gmTypes/eigen.hh>
#include <gmCore/OFactory.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   A renderer that draws single textured sphere. Use this to map video
   and images into the 3D space. The mapping is defined by a
   CoordinatesMapper.

   Typical usage:

   ~~~~~{.xml}
   <SphereRenderer
       radius="1">
     <AngularFisheyeCoordinatesMapper
         coverageAngle="3.1416"/>
     <ImageTexture
         file="fulldome180-coordinates.png"/>
   </SphereRenderer>
   ~~~~~
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

     \b XML-attribute: \c position
  */
  void setPosition(Eigen::Vector3f p);

  /**
     Set the orientation of the sphere.

     \b XML-attribute: \c orientation
  */
  void setOrientation(Eigen::Quaternionf q);

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
