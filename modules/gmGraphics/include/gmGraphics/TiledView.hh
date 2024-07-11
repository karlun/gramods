
#ifndef GRAMODS_GRAPHICS_TILEDVIEW
#define GRAMODS_GRAPHICS_TILEDVIEW

// Required before gmCore/OFactory.hh for some compilers
#include <gmCore/io_size.hh>

#include <gmGraphics/View.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   The TiledView tiles up a view into sub tiles each containing a
   separate View.

   Example usage:
   ~~~~~{.xml}
   <SomeWindow>
     <TiledView>
       <param name="tileLocation" value="0 2 1 1"/>
       <SomeView ... />
       <param name="tileLocation" value="1 2 1 1"/>
       <SomeView ... />
       <param name="tileLocation" value="0 0 2 2"/>
       <SomeView ... />
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
  void renderFullPipeline(ViewSettings settings) override;

  /**
     Propagates the specified visitor.

     @see Object::Visitor
  */
  void traverse(Visitor *visitor) override;

  /**
     Adds a location and span that is used when adding views. Format
     is [row col rowspan colspan] where row and col start at zero at
     top left corner.

     \gmXmlTag{gmGraphics,TiledView,tileLocation}
  */
  void addTileLocation(gmCore::size4 c);

  /**
     Adds a view to the previously specified tile location.

     \gmXmlTag{gmGraphics,TiledView,view}
  */
  void addView(std::shared_ptr<View> view);

  GM_OFI_DECLARE;

private:

  struct Impl;
  std::unique_ptr<Impl> _impl;
};

END_NAMESPACE_GMGRAPHICS;

#endif
