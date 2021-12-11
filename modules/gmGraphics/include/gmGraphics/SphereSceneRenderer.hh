
#ifndef GRAMODS_GRAPHICS_SPHERESCENERENDERER
#define GRAMODS_GRAPHICS_SPHERESCENERENDERER

#include <gmGraphics/Renderer.hh>

#include <gmCore/eigen.hh>
#include <gmCore/OFactory.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   A renderer that draws a set of spheres, for demonstration and
   testing purposes.
*/
class SphereSceneRenderer
  : public Renderer {

public:

  SphereSceneRenderer();

  /**
     Performs rendering of 3D objects in the scene.
  */
  void render(Camera camera, float near = -1, float far = -1) override;

  /**
     Extracts the currently optimal near and far plane distances. This
     is typically used by a View if there are multiple renderers that
     need to be rendered with the same near and far planes for correct
     depth testing.
  */
  void getNearFar(Camera camera, float &near, float &far) override;

  /**
     Sets the radius of each sphere to be drawn.

     \gmXmlTag{gmGraphics,SphereSceneRenderer,sphereRadius}
  */
  void setSphereRadius(float r);

  /**
     Sets the radius of the set of spheres.

     \gmXmlTag{gmGraphics,SphereSceneRenderer,sphereSetRadius}
  */
  void setSphereSetRadius(float r);

  /**
     Sets the position of the sphere set.

     \gmXmlTag{gmGraphics,SphereSceneRenderer,position}

     \sa gramods::operator>>(std::istream &, Eigen::Vector3f &)
  */
  void setPosition(Eigen::Vector3f c);

  /**
     Sets the color of the spheres in the set. Shading will be added.

     \sa gramods::operator>>(std::istream &, Eigen::Vector3f &)
  */
  void setColor(Eigen::Vector3f c);

  GM_OFI_DECLARE;

private:

  struct Impl;
  std::unique_ptr<Impl> _impl;

};

END_NAMESPACE_GMGRAPHICS;

#endif
