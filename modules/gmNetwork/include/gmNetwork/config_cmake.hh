
#ifndef GRAMODS_NETWORK_CONFIG
#define GRAMODS_NETWORK_CONFIG

#define BEGIN_NAMESPACE_GMNETWORK namespace gramods { namespace gmNetwork {//
#define END_NAMESPACE_GMNETWORK }}//
#define USING_NAMESPACE_GMNETWORK using namespace gramods::gmNetwork

#cmakedefine gramods_ACTIVATE_ASIO_HANDLER_TRACKING
#ifdef gramods_ACTIVATE_ASIO_HANDLER_TRACKING
#define ASIO_ENABLE_HANDLER_TRACKING
#endif

#endif
