
#include <gmTrack/VRPNTracker.hh>

#ifdef GM_ENABLE_VRPN

#include <gmCore/Console.hh>

BEGIN_NAMESPACE_GMTRACK;

VRPNTracker::VRPNTracker() {}

VRPNTracker::~VRPNTracker() {
  tracker = nullptr;
}

void VRPNTracker::setConnectionString(std::string id) {
  tracker = std::make_unique<vrpn_Tracker_Remote>(id.c_str());
  tracker->register_change_handler(this, VRPNTracker::handler);
}

bool VRPNTracker::getPose(std::map<int, PoseSample> &p) {

  if (!tracker) {
    GM_WRN("Tracker", "Cannot get pose - no vrpn connection");
    return false;
  }

  if (!tracker->connectionPtr()->doing_okay()) {
    GM_WRN("Tracker", "Defunct connection - closing vrpn connection");
    tracker = nullptr;
    return false;
  }

  do {
    got_data = false;
    tracker->mainloop();
  } while (got_data);

  p = latest_samples;

  return true;
}

void VRPN_CALLBACK VRPNTracker::handler(void *data, const vrpn_TRACKERCB info) {
  VRPNTracker *_this = static_cast<VRPNTracker*>(data);

  auto secs = std::chrono::duration_cast<clock::duration>
    (std::chrono::seconds(info.msg_time.tv_sec));
  auto usecs = std::chrono::duration_cast<clock::duration>
    (std::chrono::microseconds(info.msg_time.tv_usec));
  auto time = clock::time_point(secs + usecs);

  _this->latest_samples[info.sensor].time = time;
  _this->latest_samples[info.sensor].position =
    Eigen::Vector3f(info.pos[0], info.pos[1], info.pos[2]);
  _this->latest_samples[info.sensor].orientation =
    Eigen::Quaternionf(info.quat[0], info.quat[1], info.quat[2], info.quat[3]);

  _this->got_data = true;
  GM_VINF("Tracker", "Got vrpn tracker data for sensor " << info.sensor);
}

END_NAMESPACE_GMTRACK;

#endif
