
#ifndef GRAMODS_NETWORK_CONFIG
#define GRAMODS_NETWORK_CONFIG

#define BEGIN_NAMESPACE_GMNETWORK namespace gramods { namespace gmNetwork {//
#define END_NAMESPACE_GMNETWORK }}//
#define USING_NAMESPACE_GMNETWORK using namespace gramods::gmNetwork

namespace gramods {

  /**
     The gmNetwork module provides network data synchronization and
     execution synchronization.
  */
  namespace gmNetwork {}
}


#include <gmCore/config.hh>


// Increment this when making braking changes to how gmNetwork
// communicates.
#define GRAMODS_NETWORK_VERSION 0

#cmakedefine _WIN32_WINNT @_WIN32_WINNT@

#cmakedefine gramods_ACTIVATE_ASIO_HANDLER_TRACKING
#ifdef gramods_ACTIVATE_ASIO_HANDLER_TRACKING
#  ifndef ASIO_ENABLE_HANDLER_TRACKING
#    define ASIO_ENABLE_HANDLER_TRACKING
#  endif
#endif

#endif
