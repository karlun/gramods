
#ifndef GRAMODS_GRAPHICS_FOVPLANARVIEW
#define GRAMODS_GRAPHICS_FOVPLANARVIEW

#include <gmGraphics/View.hh>

#include <gmTypes/float.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   This view provides skewed frustum projection of the renderers.
*/
class FovPlanarView
  : public View {

public:

  void renderFullPipeline(ViewSettings settings);

  /**
     Sets the clip planes, left, right, bottom, top, in x, y
     coordinates at distance 1.

     \b XML-attribute: \c clipPlanes
  */
  void setClipPlanes(gmTypes::float4 p) {
    planes = p;
  }

  /**
     Angles between forward direction and the clip planes left, right,
     bottom and top, in radians. Positive angles are right and up.

     \b XML-attribute: \c clipAngles
  */
  void setClipAngles(gmTypes::float4 p) {
    planes[0] = tan(p[0]);
    planes[1] = tan(p[1]);
    planes[2] = tan(p[2]);
    planes[3] = tan(p[3]);
  }

  /**
     Sets the orientation of the View.

     \b XML-attribute: \c orientation
  */
  void setOrientation(Eigen::Quaternionf q) {
    orientation = q;
  }

  GM_OFI_DECLARE;

private:

  gmTypes::float4 planes = { -1, 1, -1, 1 };
  Eigen::Quaternionf orientation = Eigen::Quaternionf::Identity();

};

END_NAMESPACE_GMGRAPHICS;

#endif
