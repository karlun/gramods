
#ifndef GRAMODS_TRACK_FLOATVRPNTRACKER
#define GRAMODS_TRACK_FLOATVRPNTRACKER

#include <gmTrack/VrpnTracker.hh>

#ifdef gramods_ENABLE_VRPN

#include <gmCore/io_eigen.hh>
#include <gmCore/OFactory.hh>

#include <vrpn_Analog.h>

BEGIN_NAMESPACE_GMTRACK;

/**
   Float tracker reading data off a VRPN server.

   @see VrpnTracker
*/
class FloatVrpnTracker
  : public VrpnTracker<float, vrpn_Analog_Remote, vrpn_ANALOGCB> {

public:

  GM_OFI_DECLARE;
};

END_NAMESPACE_GMTRACK;

#endif
#endif
