
#ifndef GRAMODS_GRAPHICS_MIXINGVIEW
#define GRAMODS_GRAPHICS_MIXINGVIEW

#include <gmCore/OFactory.hh>

#include <gmGraphics/View.hh>
#include <gmGraphics/Geometry.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   This view mixes the graphics from two or more other views.
*/
class MixingView : public View {

public:
  MixingView();
  virtual ~MixingView();

  /**
     @see ViewBase::renderFullPipeline.
  */
  void renderFullPipeline(ViewSettings settings) override;

  /**
     Propagates the specified visitor.

     @see Object::Visitor
  */
  void traverse(Visitor *visitor) override;

  /**
     Set type of mixing. Default is average.

     \gmXmlTag{gmGraphics,MixingView,mixType}

     @see MixingShaders
  */
  void setMixType(std::string);

  /**
     Add a view to mix.

     \gmXmlTag{gmGraphics,MixingView,view}
  */
  void addView(std::shared_ptr<View>);

  GM_OFI_DECLARE;

private:
  struct Impl;
  std::unique_ptr<Impl> _impl;
};

END_NAMESPACE_GMGRAPHICS;

#endif
