
#ifndef GRAMODS_TRACK_POSEVRPNTRACKER
#define GRAMODS_TRACK_POSEVRPNTRACKER

#include <gmTrack/VrpnTracker.hh>

#ifdef gramods_ENABLE_VRPN

#include <gmCore/io_eigen.hh>
#include <gmCore/OFactory.hh>

#include <vrpn_Tracker.h>

BEGIN_NAMESPACE_GMTRACK;

/**
   Pose tracker reading data off a VRPN server.

   @see VrpnTracker
*/
class PoseVrpnTracker
  : public VrpnTracker<gmCore::Pose, vrpn_Tracker_Remote, vrpn_TRACKERCB> {

public:

  GM_OFI_DECLARE;
};

END_NAMESPACE_GMTRACK;

#endif
#endif
