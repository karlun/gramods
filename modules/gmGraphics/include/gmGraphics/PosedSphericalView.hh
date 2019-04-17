
#ifndef GRAMODS_GRAPHICS_POSEDSPHERICALVIEW
#define GRAMODS_GRAPHICS_POSEDSPHERICALVIEW

#include <gmGraphics/View.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   The PosedSphericalView maps the renderers' graphics into one
   selected representation of spherical projection, from the
   viewpoint's position and orientation. Supported projections are
   equirectangular and angular fisheye.
*/
class PosedSphericalView
  : public View {

public:

  PosedSphericalView();

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
     Sets the vertical coverage of the angular fisheye or horizontal
     for the equirectangular projection, in radians. Default is
     2π.
  */
  void setCoverageAngle(float a);

  /**
     Selects projection between equirectangular (0), angular
     fisheye (1) and square angular fisheye (2). Default is 0.
  */
  void setProjectionType(int p);

  GM_OFI_DECLARE;

private:

  class Impl;
  std::unique_ptr<Impl> _impl;

};

END_NAMESPACE_GMGRAPHICS;

#endif
