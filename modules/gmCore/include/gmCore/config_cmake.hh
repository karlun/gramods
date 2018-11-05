
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

#ifdef WIN32
#ifdef gmCore_EXPORTS
#define gmCore_API __declspec(dllexport)
#else
#define gmCore_API __declspec(dllimport)
#endif
#else
#define gmCore_API
#endif

#endif
