
#ifndef GRAMODS_GRAPHICS_PLANEGEOMETRY
#define GRAMODS_GRAPHICS_PLANEGEOMETRY

#include <gmTypes/all.hh>
#include <gmGraphics/Geometry.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   PlaneGeometry contains information for projection of graphics on a
   plane. This can be used to compensate for keystoning.
*/
class PlaneGeometry
  : public gmGraphics::Geometry {

public:

  PlaneGeometry();
  virtual ~PlaneGeometry();

  /**
     Set the position of the plane.
  */
  void setPosition(gmTypes::float3);

  /**
     Set the orientation of the plane by defining its normal.
  */
  void setNormal(gmTypes::float3);

  /**
     Set the orientation of the plane as quaternion in format (w x y
     z). The zero rotation plane is in the x-y plane.
  */
  void setQuaternion(gmTypes::float4);

  /**
     Set the orientation of the plane as axis angle in format (x y z
     a), where angle a is expressed in radians. The zero rotation
     plane is in the x-y plane.
  */
  void setAxisAngle(gmTypes::float4);

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
                             Camera &rfrustum);

  /**
     Returns shader code that implements the necessary functions for
     the specific Geometry.

     The code implements

     - ```vec3 getIntersection(vec3 pos, vec3 dir)``` - returns the
       intersection between the line defined by the specified position
       and direction vector and the PlaneGeometry. Return (0,0,0) if
       there is no intersection.
  */
  std::string getMapperCode();

  /**
     Called by the code that is using this geometry object, to let the
     it set the uniforms used by the mapper code.
  */
  void setMapperUniforms(GLuint program);

  GM_OFI_DECLARE;

private:

  struct Impl;

};

END_NAMESPACE_GMGRAPHICS;

#endif
