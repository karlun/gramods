
#ifndef GRAMODS_GRAPHICS_CHROMAKEYVIEW
#define GRAMODS_GRAPHICS_CHROMAKEYVIEW

// Required before gmCore/OFactory.hh for some compilers
#include <gmCore/io_float.hh>

#include <gmGraphics/View.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   The ChromaKeyView filters another view making it transparent where
   the chroma key is matched within the CbCr color space within a
   specified tolerance.
*/
class ChromaKeyView
  : public View {

public:

  ChromaKeyView();
  virtual ~ChromaKeyView();

  /**
     Appends local renderers to the view settings, forwards rendering
     to the sub views and mixes the results.
  */
  void renderFullPipeline(ViewSettings settings) override;

  /**
     Propagates the specified visitor.

     @see Object::Visitor
  */
  void traverse(Visitor *visitor) override;

  /**
     Adds a view to mask with chroma key.

     \gmXmlTag{gmGraphics,ChromaKeyView,view}
  */
  void addView(std::shared_ptr<View>);

  /**
     Set the RGB key to mask with. Default is [0, 1, 0].

     \gmXmlTag{gmGraphics,ChromaKeyView,key}
  */
  void setKey(gmCore::float3 key);

  /**
     Set the minimum and maximum tolerances against the key when
     masking. This will specify a ramp in decimal CrCb (2D) color
     space. Default is [0.48, 0.50].

     \gmXmlTag{gmGraphics,ChromaKeyView,tolerance}
  */
  void setTolerance(gmCore::float2 tol);

  GM_OFI_DECLARE;

private:

  struct Impl;
  std::unique_ptr<Impl> _impl;
};

END_NAMESPACE_GMGRAPHICS;

#endif
