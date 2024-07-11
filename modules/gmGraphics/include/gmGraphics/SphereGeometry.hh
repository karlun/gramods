
#ifndef GRAMODS_GRAPHICS_SPHEREGEOMETRY
#define GRAMODS_GRAPHICS_SPHEREGEOMETRY

// Required before gmCore/OFactory.hh for some compilers
#include <gmCore/io_eigen.hh>

#include <gmGraphics/Geometry.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   SphereGeometry contains information for projection of graphics on a
   sphere. This can be used for view correction of rendering in dome
   theatres.
*/
class SphereGeometry
  : public gmGraphics::Geometry {

public:

  SphereGeometry();
  virtual ~SphereGeometry();

  /**
     Set the position of the sphere origin, its center.

     \gmXmlTag{gmGraphics,SphereGeometry,position}

     \sa gramods::operator>>(std::istream &, Eigen::Vector3f &)
  */
  void setPosition(Eigen::Vector3f p);

  /**
     Set the radius of the sphere.

     \gmXmlTag{gmGraphics,SphereGeometry,radius}
  */
  void setRadius(float);

  /**
     Until the render frustum estimation has been implemented to
     correctly cover the full view frustum, this frustum size
     modulation parameter can be used to enlarge the render
     frustum. Use this if you see D-shaped black regions at the edge
     of the view. Default is 1.0.

     \gmXmlTag{gmGraphics,SphereGeometry,frustumSizeRatio}
  */
  void setFrustumSizeRatio(float);

  /**
     Calculates and returns a frustum, with optical center at the
     specified position, that contains the intersection between the
     provided frustum and the geometrical shape.

     @param[in] vfrustum The view frustum, typically the frustum of a
     projector that projects onto a geometrical surface.

     @param[in] position The rendering position, that will be the
     position of the returned render frustum.

     @param[out] rfrustum The render frustum, a frustum that will
     provide a rendering of the scene, from the provided position,
     onto the geometry where this geometry is seen by the vfrustum
     frustum.

     @returns True if a camera could be specified and false if there
     is no valid camera that fulfills the requirements.
  */
  bool getCameraFromPosition(Camera vfrustum,
                             Eigen::Vector3f position,
                             Camera &rfrustum) override;

  /**
     Returns shader code that implements the necessary functions for
     the specific Geometry.

     The code implements

     - ```vec3 getIntersection(vec3 pos, vec3 dir)``` - returns the
       intersection between the line defined by the specified position
       and direction vector and the SphereGeometry. Return (0,0) if there is
       no intersection.
  */
  std::string getIntersectionCode() override;

  /**
     Called by the code that is using this geometry object, to let the
     it set the uniforms used by the mapper code.
  */
  void setMapperUniforms(GLuint program) override;

  GM_OFI_DECLARE;

private:

  struct Impl;

};

END_NAMESPACE_GMGRAPHICS;

#endif
