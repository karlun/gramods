
#ifndef GRAMODS_GRAPHICS_SPHERERENDERER
#define GRAMODS_GRAPHICS_SPHERERENDERER

#include <gmGraphics/Renderer.hh>
#include <gmGraphics/Texture.hh>

#include <gmTypes/all.hh>
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

  void setRadius(float r);
  void setCenter(gmTypes::float3 c);

  /**
     Sets the Texture to apply on the sphere.
  */
  void setTexture(std::shared_ptr<Texture> tex);

  /**
     Sets the vertical coverage of the angular fisheye texture, in
     radians. Default is 2π.
  */
  void setTextureCoverageAngle(float v);

  GM_OFI_DECLARE;

private:

  class Impl;
  std::unique_ptr<Impl> _impl;

};

END_NAMESPACE_GMGRAPHICS;

#endif
