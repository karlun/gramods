
#ifndef GRAMODS_MISC_CONFIG
#define GRAMODS_MISC_CONFIG

#define BEGIN_NAMESPACE_GMMISC namespace gramods { namespace gmMisc { //
#define END_NAMESPACE_GMMISC }}//
#define USING_NAMESPACE_GMMISC using namespace gramods::gmMisc; //

namespace gramods {

  /**
     The miscellaneous module (*misc*) contains functionality that
     cannot be categorized in a good way that would not result in one
     module for each class.
  */
  namespace gmMisc {}
}


#include <gmCore/config.hh>

#cmakedefine HAVE_fftw
#ifdef HAVE_fftw
#cmakedefine gramods_ENABLE_fftw
#endif

#cmakedefine HAVE_Lehdari_Delaunay
#ifdef HAVE_Lehdari_Delaunay
#cmakedefine gramods_ENABLE_Lehdari_Delaunay
#endif


#endif
