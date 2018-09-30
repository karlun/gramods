
#ifndef GRAMODS_GRAPHICS_SPATIALPLANARVIEW
#define GRAMODS_GRAPHICS_SPATIALPLANARVIEW

#include <gmGraphics/View.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   This view provides skewed frustum projection of the renderers.
*/
class SpatialPlanarView
  : public View {

public:

  void renderFullPipeline(ViewSettings settings);

  void setTopLeftCorner(gmTypes::float3 tlc) {
    topLeftCorner = Eigen::Vector3f(tlc[0], tlc[1], tlc[2]);
  }
  void setBottomRightCorner(gmTypes::float3 brc) {
    bottomRightCorner = Eigen::Vector3f(brc[0], brc[1], brc[2]);
  }
  void setUpDirection(gmTypes::float3 up) {
    upDirection = Eigen::Vector3f(up[0], up[1], up[2]);
  }

  GM_OFI_DECLARE;

private:

  Eigen::Vector3f topLeftCorner;
  Eigen::Vector3f bottomRightCorner;
  Eigen::Vector3f upDirection;

};

END_NAMESPACE_GMGRAPHICS;

#endif
