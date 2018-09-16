
#ifndef GRAMODS_GRAPHICS_CONFIG
#define GRAMODS_GRAPHICS_CONFIG

#define BEGIN_NAMESPACE_GMGRAPHICS namespace gramods { namespace gmGraphics {//
#define END_NAMESPACE_GMGRAPHICS }}//
#define USING_NAMESPACE_GMGRAPHICS using namespace gramods::gmGraphics;//


#cmakedefine SDL2_FOUND
#ifdef SDL2_FOUND
#cmakedefine gramods_ENABLE_SDL2
#endif


#endif
