
#ifndef GRAMODS_GRAPHICS_SPATIALPLANARVIEW
#define GRAMODS_GRAPHICS_SPATIALPLANARVIEW

#include <gmGraphics/MultiscopicView.hh>

#include <gmTypes/eigen.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   This view provides skewed frustum projection of the renderers.
*/
class SpatialPlanarView
  : public MultiscopicView {

public:

  /**
     Position of the top left corner of the view.

     \b XML-attribute: \c topLeftCorner
  */
  void setTopLeftCorner(Eigen::Vector3f tlc) {
    topLeftCorner = tlc;
  }

  /**
     Position of bottom right corner of the view.

     \b XML-attribute: \c bottomRightCorner
  */
  void setBottomRightCorner(Eigen::Vector3f brc) {
    bottomRightCorner = brc;
  }

  /**
     Set up direction of the view.

     \b XML-attribute: \c upDirection
  */
  void setUpDirection(Eigen::Vector3f up) {
    upDirection = up.normalized();
  }

  GM_OFI_DECLARE;

private:

  void renderFullPipeline(ViewSettings settings, Eye eye);

  Eigen::Vector3f topLeftCorner;
  Eigen::Vector3f bottomRightCorner;
  Eigen::Vector3f upDirection;

};

END_NAMESPACE_GMGRAPHICS;

#endif
