
#ifndef GRAMODS_GRAPHICS_VIEWMIXVIEW
#define GRAMODS_GRAPHICS_VIEWMIXVIEW

#include <gmGraphics/View.hh>

#include <gmGraphics/Geometry.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   This view mixes the graphics from two or more other views.
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
     Set type of mixing. Default is average. Valid values are

     - average, showing the per pixel average value of all the views,

     - difference, showing the per pixel difference between the first
       and the second added view.

     - distance, showing the per pixel color distance between the
       first and the second added view.

     \b XML-attribute: \c mixType
  */
  void setMixType(std::string);

  /**
     Add a view to mix.

     \b XML-key: \c view
  */
  void addView(std::shared_ptr<View>);

  GM_OFI_DECLARE;

private:

  struct Impl;
  std::unique_ptr<Impl> _impl;
};

END_NAMESPACE_GMGRAPHICS;

#endif
