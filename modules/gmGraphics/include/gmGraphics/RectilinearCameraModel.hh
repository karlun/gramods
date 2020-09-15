
#ifndef GRAMODS_GRAPHICS_RECTILINEARCAMERAMODEL
#define GRAMODS_GRAPHICS_RECTILINEARCAMERAMODEL

#include <gmGraphics/CoordinatesMapper.hh>

#include <gmTypes/float.hh>

#include <GL/glew.h>
#include <GL/gl.h>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   Coordinates mapper that compensates for the optical effects as
   described by a rectilinear camera model.
*/
class RectilinearCameraModel
  : public gmGraphics::CoordinatesMapper {

public:

  RectilinearCameraModel();
  ~RectilinearCameraModel();

  /**
     Returns shader code that implements the necessary functions for
     the specific coordinates mapper.

     The code implements ```bool mapTo2D(vec3 pos3, out vec2 pos2)```
     that calculates the 2D coordinates that correspond to the
     specified 3D position. Return true if pos2 is set to the 2D
     mapping of pos3, false if there is no such mapping.
  */
  std::string getMapperCode();

  /**
     Called by the code that is using this CoordinatesMapper object,
     to let it set the uniforms used by the mapper code.
  */
  void setMapperUniforms(GLuint program);

  /**
     Set the k distortion parameters (k1 k2 k3).

     \b XML-key: \c kDistortion
  */
  void setKDistortion(gmTypes::float3 k);

  /**
     Set the p distortion parameters (p1 p2).

     \b XML-key: \c pDistortion
  */
  void setPDistortion(gmTypes::float2 p);

  /**
     Set the focal distance (fx fy) for the camera model, expressed
     relative to the image size. Calibration output from OpenCV is
     typically expressed in pixels so divide by the pixel size in each
     dimension, respectively, before entering values here.

     To convert from focal distance in mm and sensor ratio, multiply
     the focal distance of the lens by the sensor ratio (e.g. 28 mm x
     1.6 = 44.8 mm) and calculate 36 divided by this value (e.g. 36 /
     44.8 ≈ 0.8). This will be the focal distance for your largest
     dimension. For the other dimension you divide the previous value
     with the image ratio (e.g. 0.8 / (4/3) = 0.6).

     \b XML-key: \c focalDistance
  */
  void setFocalDistance(gmTypes::float2 f);

  /**
     Set the focal offset (cx cy) for the camera model, expressed
     relative to the image size. A straight projection should have an
     offset close to (0.5 0.5). Calibration output from OpenCV is
     typically expressed in pixels so divide by resolution before
     entering values here.

     \b XML-key: \c focalOffset
  */
  void setFocalOffset(gmTypes::float2 c);

  GM_OFI_DECLARE;

private:

  struct Impl;
  std::unique_ptr<Impl> _impl;
};

END_NAMESPACE_GMGRAPHICS;

#endif
