
#ifndef GRAMODS_GRAPHICS_POSEDPLANARVIEW
#define GRAMODS_GRAPHICS_POSEDPLANARVIEW

// Required before gmCore/OFactory.hh for some compilers
#include <gmCore/io_angle.hh>

#include <gmGraphics/View.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   This view provides a projection of the renderers using the
   specified frustum projection from the viewpoint's point of view.
*/
class PosedPlanarView
  : public View {

public:

  PosedPlanarView();

  void renderFullPipeline(ViewSettings settings) override;

  /**
     Sets the horizontal and vertical field of view, in radians. If
     one dimension is set to -1 then that dimension is automatically
     controlled to fit the ratio of the viewport. Default is 1
     horizontal.

     \gmXmlTag{gmGraphics,PosedPlanarView,fieldOfView}

     \sa operator>>(std::istream &, gmCore::angle &)
  */
  void setFieldOfView(gmCore::angle2 fov);

  GM_OFI_DECLARE;

private:

  struct Impl;
  std::unique_ptr<Impl> _impl;
};

END_NAMESPACE_GMGRAPHICS;

#endif
