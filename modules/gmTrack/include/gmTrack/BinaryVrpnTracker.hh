
#ifndef GRAMODS_TRACK_BINARYVRPNTRACKER
#define GRAMODS_TRACK_BINARYVRPNTRACKER

#include <gmTrack/VrpnTracker.hh>

#ifdef gramods_ENABLE_VRPN

#include <gmCore/io_eigen.hh>
#include <gmCore/OFactory.hh>

#include <vrpn_Button.h>

BEGIN_NAMESPACE_GMTRACK;

/**
   Binary tracker reading data off a VRPN server.

   @see VrpnTracker
*/
class BinaryVrpnTracker
  : public VrpnTracker<bool, vrpn_Button_Remote, vrpn_BUTTONCB> {

public:

  GM_OFI_DECLARE;
};

END_NAMESPACE_GMTRACK;

#endif
#endif
