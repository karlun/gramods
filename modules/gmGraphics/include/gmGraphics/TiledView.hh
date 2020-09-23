
#ifndef GRAMODS_GRAPHICS_TILEDVIEW
#define GRAMODS_GRAPHICS_TILEDVIEW

#include <gmGraphics/View.hh>

#include <gmTypes/size.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   The TiledView tiles up a RendererDispatcher (a Window or another
   View) into sub tiles each containing a separate View.

   Example usage:
   ~~~~~{.xml}
   <SomeWindow>
     <TiledView>
       <param name="tileLocation" value="0 2 1 1"/>
       <SomeView AS="view"/>
       <param name="tileLocation" value="1 2 1 1"/>
       <SomeView AS="view"/>
       <param name="tileLocation" value="0 0 2 2"/>
       <SomeView AS="view"/>
     </TiledView>
   </SdlWindow>
   ~~~~~
*/
class TiledView
  : public View {

public:

  TiledView();

  /**
     Appends local renderers to the view settings and forwards
     rendering to the sub views of this tiled view.
  */
  void renderFullPipeline(ViewSettings settings);

  /**
     Adds a location and span that is used when adding views. Format
     is [row col rowspan colspan] where row and col start at zero at
     bottom left corner.

     \gmXmlTag{gmGraphics,TiledView,tileLocation}
  */
  void addTileLocation(gmTypes::size4 c);

  /**
     Adds a view to the previously specified tile location.

     \gmXmlTag{gmGraphics,TiledView,view}
  */
  void addView(std::shared_ptr<View> view);

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
