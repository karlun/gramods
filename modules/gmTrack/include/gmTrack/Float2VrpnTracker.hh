
#ifndef GRAMODS_TRACK_FLOAT2VRPNTRACKER
#define GRAMODS_TRACK_FLOAT2VRPNTRACKER

#include <gmTrack/VrpnTracker.hh>

#ifdef gramods_ENABLE_VRPN

#include <gmCore/io_eigen.hh>
#include <gmCore/OFactory.hh>

#include <vrpn_Analog.h>

BEGIN_NAMESPACE_GMTRACK;

/**
   Float2 tracker reading data off a VRPN server.

   @see VrpnTracker
*/
class Float2VrpnTracker
  : public VrpnTracker<gmCore::float2, vrpn_Analog_Remote, vrpn_ANALOGCB> {

public:

  GM_OFI_DECLARE;
};

END_NAMESPACE_GMTRACK;

#endif
#endif
