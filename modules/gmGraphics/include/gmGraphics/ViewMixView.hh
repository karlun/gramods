
#ifndef GRAMODS_GRAPHICS_VIEWMIXVIEW
#define GRAMODS_GRAPHICS_VIEWMIXVIEW

#include <gmGraphics/StereoscopicView.hh>

#include <gmGraphics/Geometry.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   This view mixes the graphics from two or more other views. The
   current implementation shows the per pixel difference between the
   first added view and the other added views.
*/
class ViewMixView
  : public View {

public:

  ViewMixView();
  virtual ~ViewMixView();

  /**
     Appends local renderers to the view settings, forwards rendering
     to the sub views and mixes the results.
  */
  void renderFullPipeline(ViewSettings settings);

  /**
     Add a view to mix.
  */
  void addView(std::shared_ptr<View>);

  GM_OFI_DECLARE;

private:

  struct Impl;
  std::unique_ptr<Impl> _impl;
};

END_NAMESPACE_GMGRAPHICS;

#endif
