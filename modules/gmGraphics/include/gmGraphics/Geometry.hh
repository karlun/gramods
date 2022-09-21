
#ifndef GRAMODS_GRAPHICS_GEOMETRY
#define GRAMODS_GRAPHICS_GEOMETRY

#include <gmCore/Object.hh>
#include <gmCore/OFactory.hh>

#include <gmGraphics/Camera.hh>

#include <GL/glew.h>
#include <GL/gl.h>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   A Geometry contains information for projection of graphics on a
   geometrical shape, such as a sphere or a plane.
*/
class Geometry
  : public gmCore::Object {

protected:

  struct Impl;
  std::unique_ptr<Impl> _impl;

public:

  Geometry(Impl * _impl = nullptr);
  virtual ~Geometry();

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
     is no valid camera that fulfills the requirements, for example if
     the view because of non-linearities in the geometry would result
     in a field-of-view of 180 degrees or more.
  */
  virtual bool getCameraFromPosition(Camera vfrustum,
                                     Eigen::Vector3f position,
                                     Camera &rfrustum);

  /**
     Returns shader code that implements the necessary functions for
     the specific geometry.

     The code must implement

     - ```vec3 getIntersection(vec3 pos, vec3 dir)``` - returns the
       intersection between the line defined by the specified position
       and direction vector and the geometry. Return (0,0,0) if there
       is no intersection.
  */
  virtual std::string getIntersectionCode() { return ""; }

  /**
     Called by the code that is using this geometry object, to let the
     it set the uniforms used by the mapper code.
  */
  virtual void setMapperUniforms(GLuint) {}

  /**
     Set true if the projection should be calculated for the inside of
     the geometry, and not the outside. Default is false (projection
     is estimated for the outside of the geometry).

     \gmXmlTag{gmGraphics,Geometry,inside}
  */
  void setInside(bool on);

  /**
     Returns the default key, in Configuration, for the
     Object.
  */
  virtual std::string getDefaultKey() override { return "geometry"; }

  GM_OFI_DECLARE;

};

END_NAMESPACE_GMGRAPHICS;

#endif
