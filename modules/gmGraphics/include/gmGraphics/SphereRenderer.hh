
#ifndef GRAMODS_GRAPHICS_SPHERERENDERER
#define GRAMODS_GRAPHICS_SPHERERENDERER

#include <gmGraphics/Renderer.hh>
#include <gmGraphics/TextureInterface.hh>
#include <gmGraphics/CoordinatesMapper.hh>

#include <gmCore/io_eigen.hh>
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
  void render(const Camera &camera, const Eigen::Affine3f &Mm) override;

  /**
     Extracts the currently optimal near and far plane distances. This
     is typically used by a View if there are multiple renderers that
     need to be rendered with the same near and far planes for correct
     depth testing.
  */
  void getNearFar(const Camera &camera,
                  const Eigen::Affine3f &Mm,
                  float &near,
                  float &far) override;

  /**
     Propagates the specified visitor.

     @see Object::Visitor
  */
  void traverse(Visitor *visitor) override;

  /**
     Set the radius of the sphere.

     \gmXmlTag{gmGraphics,SphereRenderer,radius}
  */
  void setRadius(float r);

  /**
     Sets the Texture to apply on the sphere.

     \gmXmlTag{gmGraphics,SphereRenderer,texture}
  */
  void setTexture(std::shared_ptr<TextureInterface> tex);

  /**
     Sets the coordinates mapper that should be used to map the 2D
     texture onto the 3D sphere.
  */
  void setCoordinatesMapper(std::shared_ptr<CoordinatesMapper> mapper);

  GM_OFI_DECLARE;

private:

  struct Impl;
  std::unique_ptr<Impl> _impl;

};

END_NAMESPACE_GMGRAPHICS;

#endif
