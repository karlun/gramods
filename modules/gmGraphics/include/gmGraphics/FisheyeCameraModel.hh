
#ifndef GRAMODS_GRAPHICS_FISHEYECAMERAMODEL
#define GRAMODS_GRAPHICS_FISHEYECAMERAMODEL

// Required before gmCore/OFactory.hh for some compilers
#include <gmCore/io_float.hh>

#include <gmGraphics/CoordinatesMapper.hh>

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
     @see CoordinatesMapper
  */
  std::string getTo2DCode() override;

  /**
     @see CoordinatesMapper
  */
  void setTo2DUniforms(GLuint program) override;

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
