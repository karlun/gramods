
#ifndef GRAMODS_GRAPHICS_ANGULARFISHEYESPATIALDOMEVIEW
#define GRAMODS_GRAPHICS_ANGULARFISHEYESPATIALDOMEVIEW

#include <gmGraphics/StereoscopicView.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   The AngularFisheyeSpatialDomeView maps the renderers' graphics into
   an angular fisheye projection for a dome surface. This skews the
   view from the viewpoint position relative the dome surface.
*/
class AngularFisheyeSpatialDomeView
  : public StereoscopicView {

public:

  AngularFisheyeSpatialDomeView();

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
     Sets the vertical coverage of the angular fisheye, in
     radians. Default is π.
  */
  void setCoverageAngle(float a);

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
