
#ifndef GRAMODS_CORE_CONFIG
#define GRAMODS_CORE_CONFIG

#define BEGIN_NAMESPACE_GMCORE namespace gramods { namespace gmCore {//
#define END_NAMESPACE_GMCORE }}//
#define USING_NAMESPACE_GMCORE using namespace gramods::gmCore;//

#cmakedefine gramods_STRIP_PATH_FROM_FILE

#cmakedefine SDL2_FOUND
#ifdef SDL2_FOUND
#cmakedefine gramods_ENABLE_SDL2
#endif

#endif
