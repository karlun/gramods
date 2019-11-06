
#ifndef GRAMODS_GRAPHICS_SPATIALSPHERICALVIEW
#define GRAMODS_GRAPHICS_SPATIALSPHERICALVIEW

#include <gmGraphics/MultiscopicView.hh>

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
  : public MultiscopicView {

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

     \b XML-attribute: \c position
  */
  void setPosition(Eigen::Vector3f p);

  /**
     Set the radius of the projection surface, in meters. Default is 10.

     \b XML-attribute: \c radius
  */
  void setRadius(float r);

  /**
     Set the orientation of the projection, for example to create a
     forward tilt common in dome theaters.

     \b XML-attribute: \c orientation
  */
  void setOrientation(Eigen::Quaternionf q);

  GM_OFI_DECLARE;

private:

  struct Impl;
  std::unique_ptr<Impl> _impl;

};

END_NAMESPACE_GMGRAPHICS;

#endif
