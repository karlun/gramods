
#ifndef GRAMODS_GRAPHICS_TILEDVIEW
#define GRAMODS_GRAPHICS_TILEDVIEW

#include <gmGraphics/View.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   The TiledView tiles up a RendererDispatcher (a Window or another
   View) into sub tiles each containing a separate View.

   Example usage:
   ```{xml}
   <SomeWindow>
     <TiledView>
       <param name="tileLocation" value="0 0 2 2"/>
       <SomeView AS="view"/>
       <param name="tileLocation" value="2 0 1 1"/>
       <SomeView AS="view"/>
       <param name="tileLocation" value="2 1 1 1"/>
       <SomeView AS="view"/>
     </TiledView>
   </SdlWindow>
   ```
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
     is [row col rowspan colspan].
   */
  void addTileLocation(gmTypes::size4 c);

  /**
     Adds a view to the previously specified tile location.
  */
  void addView(std::shared_ptr<View> view);

  GM_OFI_DECLARE;

private:

  class Impl;
  std::unique_ptr<Impl> _impl;
};

END_NAMESPACE_GMGRAPHICS;

#endif
