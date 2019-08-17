
#ifndef GRAMODS_TYPES_CONFIG
#define GRAMODS_TYPES_CONFIG

#define BEGIN_NAMESPACE_GRAMODS namespace gramods {//
#define END_NAMESPACE_GRAMODS }//
#define USING_NAMESPACE_GRAMODS using namespace gramods

#define BEGIN_NAMESPACE_GMTYPES namespace gramods { namespace gmTypes {//
#define END_NAMESPACE_GMTYPES }}//
#define USING_NAMESPACE_GMTYPES using namespace gramods::gmTypes

namespace gramods {

  /**
     Special types for use in the other modules, and their
     operators. In particular, this module defines std::array template
     instantiations for setting vector type attributes.
  */
  namespace gmTypes {}
}

#ifndef gramods_PI
#  include <cmath>
#  define gramods_PI   (            std::acos(-1))
#  define gramods_PI_2 (            std::asin( 1))
#  define gramods_PI_4 ((1.0/2.0) * std::asin( 1))
#endif


#cmakedefine HAVE_Eigen3
#ifdef HAVE_Eigen3
#cmakedefine gramods_ENABLE_Eigen3
#endif

#endif
