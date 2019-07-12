
#ifndef GRAMODS_GRAPHICS_EYE
#define GRAMODS_GRAPHICS_EYE

#include <gmGraphics/config.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   The three eyes that can be rendered. Many views can only render
   only a mono eye and some views, extending StereoscopicView, will
   also render only a mono eye unless there is a
   StereoscopicMultiplexer provided. Only then will there be left and
   right eyes rendered.

   @see StereoscopicView
   @see StereoscopicMultiplexer
   @see Camera
*/
enum struct Eye {
    MONO,
    LEFT,
    RIGHT
};

END_NAMESPACE_GMGRAPHICS;

#endif
