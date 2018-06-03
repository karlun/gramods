
#include <gmTrack/VRPNPoseTracker.hh>

#ifdef gramods_ENABLE_VRPN

#include <gmCore/Console.hh>

BEGIN_NAMESPACE_GMTRACK;

GM_OFI_DEFINE(VRPNPoseTracker);
GM_OFI_PARAM(VRPNPoseTracker, connectionString, std::string, VRPNPoseTracker::setConnectionString);

VRPNPoseTracker::VRPNPoseTracker() {}

VRPNPoseTracker::~VRPNPoseTracker() {
  tracker = nullptr;
}

void VRPNPoseTracker::setConnectionString(std::string id) {
  tracker = std::make_unique<vrpn_Tracker_Remote>(id.c_str());
  tracker->register_change_handler(this, VRPNPoseTracker::handler);
}

bool VRPNPoseTracker::getPose(std::map<int, PoseSample> &p) {

  if (!tracker) {
    GM_WRN("VRPNPoseTracker", "Cannot get pose - no vrpn connection");
    return false;
  }

  if (!tracker->connectionPtr()->doing_okay()) {
    GM_WRN("VRPNPoseTracker", "Defunct connection - closing vrpn connection");
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

void VRPN_CALLBACK VRPNPoseTracker::handler(void *data, const vrpn_TRACKERCB info) {
  VRPNPoseTracker *_this = static_cast<VRPNPoseTracker*>(data);

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
  GM_VINF("VRPNPoseTracker", "Got vrpn tracker data for sensor " << info.sensor);
}

END_NAMESPACE_GMTRACK;

#endif
