
#ifndef GRAMODS_GRAPHICS_RECTILINEARCAMERAMODEL
#define GRAMODS_GRAPHICS_RECTILINEARCAMERAMODEL

// Required before gmCore/OFactory.hh for some compilers
#include <gmCore/io_float.hh>

#include <gmGraphics/CoordinatesMapper.hh>

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
     @see CoordinatesMapper
  */
  std::string getCommonCode() override;

  /**
     @see CoordinatesMapper
  */
  std::string getTo2DCode() override;

  /**
     @see CoordinatesMapper
  */
  std::string getTo3DCode() override;

  /**
     @see CoordinatesMapper
  */
  void setCommonUniforms(GLuint program) override;

  /**
     @see CoordinatesMapper
  */
  void setTo2DUniforms(GLuint program) override;

  /**
     Set the k distortion parameters (k1 k2 k3).

     \gmXmlTag{gmGraphics,RectilinearCameraModel,kDistortion}
  */
  void setKDistortion(gmCore::float3 k);

  /**
     Set the p distortion parameters (p1 p2).

     \gmXmlTag{gmGraphics,RectilinearCameraModel,pDistortion}
  */
  void setPDistortion(gmCore::float2 p);

  /**
     Set the focal distance (fx fy) for the camera model, expressed
     relative to the image size. Calibration output from OpenCV is
     typically expressed in pixels so divide by the pixel size in each
     dimension, respectively, before entering values here.

     To convert from focal distance in mm and sensor ratio, divide the
     focal distance by the size of your sensor (e.g. 28 mm lens with
     sensor size 23.5 x 15.6 mm gives fx = 28/23.5 and fy =
     28/15.6). Use the largest sensor dimension for your largest image
     dimension. If the sensor size is unknown but your camera reports
     standard 35 mm equivalent focal distance, then instead divide
     this focal distance value by the standard sensor size, 36 x 27 mm
     (e.g. 28 mm equivalent gives fx = 28/36 and fy = 28/27). Adjust
     the smaller dimension if the image ratio does not perfectly match
     the standard ratio (4/3). If your camera only reports it actual
     focal distance (e.g. 2.2 mm in a phone camera) then multiply this
     by the reported crop factor and proceed by using this value in
     the standard sensor estimation.

     \gmXmlTag{gmGraphics,RectilinearCameraModel,focalDistance}
  */
  void setFocalDistance(gmCore::float2 f);

  /**
     Set the focal offset (cx cy) for the camera model, expressed
     relative to the image size. A straight projection should have an
     offset close to (0.5 0.5). Calibration output from OpenCV is
     typically expressed in pixels so divide by resolution before
     entering values here.

     \gmXmlTag{gmGraphics,RectilinearCameraModel,focalOffset}
  */
  void setFocalOffset(gmCore::float2 c);

  GM_OFI_DECLARE;

private:

  struct Impl;
  std::unique_ptr<Impl> _impl;
};

END_NAMESPACE_GMGRAPHICS;

#endif
