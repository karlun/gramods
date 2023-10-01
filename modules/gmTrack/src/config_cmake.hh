
#ifndef GRAMODS_TRACK_CONFIG
#define GRAMODS_TRACK_CONFIG

#define BEGIN_NAMESPACE_GMTRACK namespace gramods { namespace gmTrack {//
#define END_NAMESPACE_GMTRACK }}//
#define USING_NAMESPACE_GMTRACK using namespace gramods::gmTrack

namespace gramods {

  /**
     The gmTrack module provides primarily pose tracking clients,
     servers and filters.
  */
  namespace gmTrack {}
}

#include <gmCore/config.hh>

#cmakedefine HAVE_VRPN
#ifdef HAVE_VRPN
#cmakedefine gramods_ENABLE_VRPN
#endif

#cmakedefine HAVE_OpenCV
#ifdef HAVE_OpenCV
#cmakedefine gramods_ENABLE_OpenCV
#endif

#cmakedefine HAVE_OpenCV_objdetect
#ifdef HAVE_OpenCV_objdetect
#cmakedefine gramods_ENABLE_OpenCV_objdetect
#endif

#endif
