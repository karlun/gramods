
#ifndef GRAMODS_TRACK_VRPNTRACKER
#define GRAMODS_TRACK_VRPNTRACKER

#include <gmTrack/MultiTracker.hh>

#ifdef GM_ENABLE_VRPN

#include <vrpn_Tracker.h>
#include <thread>
#include <mutex>
#include <condition_variable>

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

  void setUpdateRate(double fps);

  /**
     Replaces the contents of p with pose data.
  */
  bool getPose(std::map<int, PoseSample> &p);

private:

  std::unique_ptr<vrpn_Tracker_Remote> tracker;
	static void VRPN_CALLBACK handler(void *userdata, const vrpn_TRACKERCB info);
  void _update();
  clock::duration frame_time;

  std::thread thread;
  std::mutex lock;
  std::condition_variable cond;
  std::map<int, PoseSample> latest_samples;
};

END_NAMESPACE_GMTRACK;

#endif

#endif
