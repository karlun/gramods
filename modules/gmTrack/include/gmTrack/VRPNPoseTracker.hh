
#ifndef GRAMODS_TRACK_VRPNPOSETRACKER
#define GRAMODS_TRACK_VRPNPOSETRACKER

#include <gmTrack/MultiPoseTracker.hh>

#ifdef gramods_ENABLE_VRPN

#include <vrpn_Tracker.h>

#include <gmCore/OFactory.hh>

BEGIN_NAMESPACE_GMTRACK;

/**
   Pose tracker reading data off a VRPN server.
*/
class VRPNPoseTracker
  : public MultiPoseTracker {

public:

  VRPNPoseTracker();
  ~VRPNPoseTracker();

  /**
     The address to the pose tracker at the VRPN server, such as
     "tracker@localhost".
  */
  void setConnectionString(std::string id);

  /**
     Replaces the contents of p with pose data.
  */
  bool getPose(std::map<int, PoseSample> &p);

  GM_OFI_DECLARE;

private:

  std::unique_ptr<vrpn_Tracker_Remote> tracker;
	static void VRPN_CALLBACK handler(void *userdata, const vrpn_TRACKERCB info);

  std::map<int, PoseSample> latest_samples;
  bool got_data;
};

END_NAMESPACE_GMTRACK;

#endif

#endif
