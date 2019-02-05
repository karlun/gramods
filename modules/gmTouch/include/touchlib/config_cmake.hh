
#ifndef __TOUCHLIB_CONFIG_HH__
#define __TOUCHLIB_CONFIG_HH__


#cmakedefine OPENSCENEGRAPH_FOUND
#ifdef OPENSCENEGRAPH_FOUND
#cmakedefine TOUCHLIB_ENABLE_OpenSceneGraph
#endif

#cmakedefine SDL2_FOUND
#ifdef SDL2_FOUND
#cmakedefine TOUCHLIB_ENABLE_SDL2
#endif

#cmakedefine TUIO_FOUND
#ifdef TUIO_FOUND
#cmakedefine TOUCHLIB_ENABLE_TUIO
#endif

#ifndef utm50_PI
#define utm50_PI (std::acos(-1))
#endif

#endif
