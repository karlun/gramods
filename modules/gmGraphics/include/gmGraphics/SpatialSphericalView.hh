
#ifndef GRAMODS_GRAPHICS_SPATIALSPHERICALVIEW
#define GRAMODS_GRAPHICS_SPATIALSPHERICALVIEW

#include <gmGraphics/StereoscopicView.hh>

#include <gmGraphics/CoordinatesMapper.hh>
#include <gmTypes/eigen.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   The SpatialSphericalView maps the renderers' graphics onto a
   spherical projection surface, a dome at a fixed position, in a
   selected representation of spherical projection, from the
   viewpoint's position and orientation. Supported projections are
   equirectangular and angular fisheye. This skews the view from the
   viewpoint position relative the dome surface, to compensate for the
   distortion due to the curved surface.
*/
class SpatialSphericalView
  : public StereoscopicView {

public:

  SpatialSphericalView();

  /**
     Dispatches renderers.
  */
  void renderFullPipeline(ViewSettings settings, Eye eye);

  /**
     Sets the resolution of the intermediate cube map, in
     pixels. Default is 2048.

     \b XML-attribute: \c cubeMapResolution
  */
  void setCubeMapResolution(int res);

  /**
     Turn on or off linear interpolation in the projection from linear
     graphics to non-linear projection space. Default is off.

     \b XML-attribute: \c linearInterpolation
  */
  void setLinearInterpolation(bool on);

  /**
     Sets the coordinates mapper that should be used to map the 3D
     surrounding into the 2D view.
  */
  void setCoordinatesMapper(std::shared_ptr<CoordinatesMapper> mapper);

  /**
     When set to true, the output on the view will be square, so that
     using a fisheye mapper would render the view as a circle instead
     of an ellipse. Default is false.
  */
  void setMakeSquare(bool on);

  /**
     Sets the location of the center of the spherical projection
     surface, in meters. Default is zero.

     \b XML-attribute: \c sphereCenter
  */
  void setSphereCenter(Eigen::Vector3f c);

  /**
     Set the radius of the projection surface, in meters. Default is 10.

     \b XML-attribute: \c sphereRadius
  */
  void setSphereRadius(float r);

  /**
     Set the forward tilt (towards -z) of the projection surface, in
     radians. Default is 0.

     \b XML-attribute: \c tiltAngle
  */
  void setTiltAngle(float a);

  GM_OFI_DECLARE;

private:

  class Impl;
  std::unique_ptr<Impl> _impl;

};

END_NAMESPACE_GMGRAPHICS;

#endif
