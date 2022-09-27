
#ifndef GRAMODS_TOUCH_CONFIG
#define GRAMODS_TOUCH_CONFIG

#define BEGIN_NAMESPACE_GMTOUCH namespace gramods { namespace gmTouch {//
#define END_NAMESPACE_GMTOUCH }}//
#define USING_NAMESPACE_GMTOUCH using namespace gramods::gmTouch

namespace gramods {

  /**
     The module provides touch handling functionality for the
     implementation of touch screen or touch table interaction.
  */
  namespace gmTouch {}
}


#include <gmMisc/config.hh>


#cmakedefine HAVE_OpenSceneGraph
#ifdef HAVE_OpenSceneGraph
#cmakedefine gramods_ENABLE_OpenSceneGraph
#endif

#cmakedefine HAVE_TUIO
#ifdef HAVE_TUIO
#cmakedefine gramods_ENABLE_TUIO
#endif

#cmakedefine HAVE_SDL2
#ifdef HAVE_SDL2
#cmakedefine gramods_ENABLE_SDL2
#endif

#endif
