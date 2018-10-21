
#ifndef GRAMODS_GRAPHICS_SPATIALSPHERICALVIEW
#define GRAMODS_GRAPHICS_SPATIALSPHERICALVIEW

#include <gmGraphics/StereoscopicView.hh>

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
  */
  void setCubeMapResolution(int res);

  /**
     Turn on or off linear interpolation in the projection from linear
     graphics to non-linear projection space. Default is off.
  */
  void setLinearInterpolation(bool on);

  /**
     Sets the vertical coverage of the angular fisheye or horizontal
     for the equirectangular projection, in radians. Default is
     2π.
  */
  void setCoverageAngle(float a);

  /**
     Selects projection between equirectangular (0) and angular
     fisheye (1). Default is 0.
  */
  void setProjection(int p);

  /**
     Sets the location of the dome center, in meters. Default is zero.
  */
  void setDomePosition(gmTypes::float3 c);

  /**
     Set the radius of the dome, in meters. Default is 10.
   */
  void setDomeRadius(float r);

  GM_OFI_DECLARE;

private:

  class Impl;
  std::unique_ptr<Impl> _impl;

};

END_NAMESPACE_GMGRAPHICS;

#endif
