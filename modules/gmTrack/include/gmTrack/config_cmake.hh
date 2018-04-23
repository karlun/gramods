
#ifndef GRAMODS_TRACK_CONFIG
#define GRAMODS_TRACK_CONFIG

#cmakedefine HAVE_VRPN
#ifdef HAVE_VRPN
#cmakedefine gramods_ENABLE_VRPN
#endif

#define BEGIN_NAMESPACE_GMTRACK namespace gramods { namespace gmTrack {//
#define END_NAMESPACE_GMTRACK }}//
#define USING_NAMESPACE_GMTRACK using namespace gramods::gmTrack

#endif
