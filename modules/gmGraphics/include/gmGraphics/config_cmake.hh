
#ifndef GRAMODS_GRAPHICS_CONFIG
#define GRAMODS_GRAPHICS_CONFIG

#define BEGIN_NAMESPACE_GMGRAPHICS namespace gramods { namespace gmGraphics {//
#define END_NAMESPACE_GMGRAPHICS }}//
#define USING_NAMESPACE_GMGRAPHICS using namespace gramods::gmGraphics;//

#define gramods_PI   (            std::acos(-1))
#define gramods_PI_2 (            std::asin( 1))
#define gramods_PI_4 ((1.0/2.0) * std::asin( 1))


#cmakedefine HAVE_SDL2
#ifdef HAVE_SDL2
#cmakedefine gramods_ENABLE_SDL2
#endif

#cmakedefine HAVE_libuvc
#ifdef HAVE_libuvc
#cmakedefine gramods_ENABLE_libuvc
#endif

#cmakedefine HAVE_FreeImage
#ifdef HAVE_FreeImage
#cmakedefine gramods_ENABLE_FreeImage
#endif


#endif
