
#ifndef GRAMODS_VIDEO_CONFIG
#define GRAMODS_VIDEO_CONFIG

#cmakedefine HAVE_libuvc
#ifdef HAVE_libuvc
#cmakedefine gramods_ENABLE_libuvc
#endif

#define BEGIN_NAMESPACE_GMVIDEO namespace gramods { namespace gmVideo {//
#define END_NAMESPACE_GMVIDEO }}//
#define USING_NAMESPACE_GMVIDEO using namespace gramods::gmVideo

#endif
