
#ifndef GRAMODS_GRAPHICS_POSEDPLANARVIEW
#define GRAMODS_GRAPHICS_POSEDPLANARVIEW

#include <gmTypes/angle.hh>
#include <gmGraphics/View.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   This view provides a projection of the renderers using the
   specified frustum projection from the viewpoint's point of view.
*/
class PosedPlanarView
  : public View {

public:

  void renderFullPipeline(ViewSettings settings);

  /**
     Sets the horizontal and vertical field of view, in radians. If
     one dimension is set to -1 then that dimension is automatically
     controlled to fit the ratio of the viewport. Default is 1
     horizontal.

     \b XML-attribute: \c fieldOfView

     \sa operator>>(std::istream &, gmTypes::angle &)
  */
  void setFieldOfView(gmTypes::angle2 fov) {
    field_of_view = fov;
  }

  GM_OFI_DECLARE;

private:

  gmTypes::angle2 field_of_view = { 1, -1 };

};

END_NAMESPACE_GMGRAPHICS;

#endif
