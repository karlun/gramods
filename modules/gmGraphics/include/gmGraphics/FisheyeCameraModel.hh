
#ifndef GRAMODS_GRAPHICS_FISHEYECAMERAMODEL
#define GRAMODS_GRAPHICS_FISHEYECAMERAMODEL

#include <gmGraphics/CoordinatesMapper.hh>

#include <gmCore/float.hh>

#include <GL/glew.h>
#include <GL/gl.h>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   Coordinates mapper that compensates for the optical effects as
   described by a fisheye camera model. The model is the fisheye
   camera model included in OpenCV.
*/
class FisheyeCameraModel
  : public gmGraphics::CoordinatesMapper {

public:

  FisheyeCameraModel();
  ~FisheyeCameraModel();

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
     Set the distortion parameters (k1 k2 k3 k4) for the camera model.

     \gmXmlTag{gmGraphics,FisheyeCameraModel,distortion}
  */
  void setDistortion(gmCore::float4 k);

  /**
     Set the focal distance (fx fy) for the camera model. Calibration
     output from OpenCV is typically expressed in pixels so divide by
     resolution before entering values here.

     \gmXmlTag{gmGraphics,FisheyeCameraModel,focalDistance}
  */
  void setFocalDistance(gmCore::float2 f);

  /**
     Set the focal offset (cx cy) for the camera model. Calibration
     output from OpenCV is typically expressed in pixels so divide by
     resolution before entering values here.

     \gmXmlTag{gmGraphics,FisheyeCameraModel,focalOffset}
  */
  void setFocalOffset(gmCore::float2 c);

  GM_OFI_DECLARE;

private:

  struct Impl;
  std::unique_ptr<Impl> _impl;
};

END_NAMESPACE_GMGRAPHICS;

#endif
