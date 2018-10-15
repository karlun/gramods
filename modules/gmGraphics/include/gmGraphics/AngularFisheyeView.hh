
#ifndef GRAMODS_GRAPHICS_ANGULARFISHEYEVIEW
#define GRAMODS_GRAPHICS_ANGULARFISHEYEVIEW

#include <gmGraphics/View.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   The AngularFisheyeView maps the renderers' graphics into an angular
   fisheye projection.
*/
class AngularFisheyeView
  : public View {

public:

  AngularFisheyeView();

  /**
     Dispatches renderers.
  */
  void renderFullPipeline(ViewSettings settings);

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

  GM_OFI_DECLARE;

private:

  class Impl;
  std::unique_ptr<Impl> _impl;

};

END_NAMESPACE_GMGRAPHICS;

#endif
