
#ifndef GRAMODS_GRAPHICS_CHROMAKEYVIEW
#define GRAMODS_GRAPHICS_CHROMAKEYVIEW

#include <gmGraphics/View.hh>
#include <gmTypes/float.hh>

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
  void renderFullPipeline(ViewSettings settings);

  /**
     Adds a view to mask with chroma key.

     \b XML-attribute: \c view
  */
  void addView(std::shared_ptr<View>);

  /**
     Set the RGB key to mask with. Default is [0, 1, 0].

     \b XML-attribute: \c key
  */
  void setKey(gmTypes::float3 key);

  /**
     Set the minimum and maximum tolerances against the key when
     masking. This will specify a ramp in decimal CrCb (2D) color
     space. Default is [0.48, 0.50].

     \b XML-attribute: \c tolerance
  */
  void setTolerance(gmTypes::float2 tol);

  /**
     Removes all renderers and, if recursive is set to true, also
     renderers added to sub dispatchers.
  */
  virtual void clearRenderers(bool recursive = false);

  GM_OFI_DECLARE;

private:

  struct Impl;
  std::unique_ptr<Impl> _impl;
};

END_NAMESPACE_GMGRAPHICS;

#endif
