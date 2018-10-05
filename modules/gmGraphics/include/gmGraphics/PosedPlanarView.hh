
#ifndef GRAMODS_GRAPHICS_POSEDPLANARVIEW
#define GRAMODS_GRAPHICS_POSEDPLANARVIEW

#include <gmGraphics/View.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   This view provides specified frustum projection from the
   viewpoint's point of view.
*/
class PosedPlanarView
  : public View {

public:

  void renderFullPipeline(ViewSettings settings);

  /**
     Sets the vertical field of view, in radians, while the horizontal
     is affected by the size of the viewport.
  */
  void setFieldOfView(float fov) {
    field_of_view = fov;
  }

  GM_OFI_DECLARE;

private:

  float field_of_view = 1;

};

END_NAMESPACE_GMGRAPHICS;

#endif
