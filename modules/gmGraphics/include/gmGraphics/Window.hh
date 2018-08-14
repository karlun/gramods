
#ifndef GRAMODS_GRAPHICS_WINDOW
#define GRAMODS_GRAPHICS_WINDOW

#include <gmGraphics/Dispatcher.hh>
#include <gmGraphics/Tile.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   The base of graphics Window implementations.
 */
class Window
  : public Dispatcher {

public:

  /**
     Asks the Window to call the specified function once for each
     camera that requires rendering for the current window. For
     simple stereo rendering, for example, the specified function will
     be called twice, but more times for full dome rendering.
   */
  void renderFullPipeline(Camera::RenderFunction func);

  /**
     Adds a tile to the window. A window without a tile will render
     nothing - it is the tiles that provide the graphics.
  */
  void addTile(Tile);

  GM_OFI_DECLARE(Window);

};

END_NAMESPACE_GMGRAPHICS;

#endif
