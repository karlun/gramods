
#ifndef GRAMODS_MISC_CONFIG
#define GRAMODS_MISC_CONFIG

#define BEGIN_NAMESPACE_GMMISC namespace gramods { namespace gmMisc { //
#define END_NAMESPACE_GMMISC }}//
#define USING_NAMESPACE_GMMISC using namespace gramods::gmMisc; //


#cmakedefine HAVE_Eigen3
#ifdef HAVE_Eigen3
#cmakedefine gramods_ENABLE_Eigen3
#endif


#endif
