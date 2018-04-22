
#ifndef GRAMODS_TRACK_VRPNTRACKER
#define GRAMODS_TRACK_VRPNTRACKER

#include <gmTrack/MultiTracker.hh>

#ifdef GM_ENABLE_VRPN

#include <vrpn_Tracker.h>

#include <gmCore/OFactory.hh>

BEGIN_NAMESPACE_GMTRACK;

/**
   VRPNTracker
 */
class VRPNTracker
  : public MultiTracker {

public:

  VRPNTracker();
  ~VRPNTracker();

  void setConnectionString(std::string id);

  /**
     Replaces the contents of p with pose data.
  */
  bool getPose(std::map<int, PoseSample> &p);

  GM_OFI_DECLARE(VRPNTracker);

private:

  std::unique_ptr<vrpn_Tracker_Remote> tracker;
	static void VRPN_CALLBACK handler(void *userdata, const vrpn_TRACKERCB info);

  std::map<int, PoseSample> latest_samples;
  bool got_data;
};

END_NAMESPACE_GMTRACK;

#endif

#endif
