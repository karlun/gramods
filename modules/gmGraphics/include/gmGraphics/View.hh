
#ifndef GRAMODS_GRAPHICS_VIEW
#define GRAMODS_GRAPHICS_VIEW

#include <gmGraphics/RendererDispatcher.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   The base of graphics View implementations. A view is a projection
   of rendered data into a buffer.
*/
class View
  : public RendererDispatcher {

public:

  GM_OFI_DECLARE;

};

END_NAMESPACE_GMGRAPHICS;

#endif
