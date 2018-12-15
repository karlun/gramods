
#include <gmTrack/VrpnPoseTracker.hh>

#ifdef gramods_ENABLE_VRPN

#include <gmCore/Console.hh>

BEGIN_NAMESPACE_GMTRACK;

GM_OFI_DEFINE(VrpnPoseTracker);
GM_OFI_PARAM(VrpnPoseTracker, connectionString, std::string, VrpnPoseTracker::setConnectionString);

VrpnPoseTracker::VrpnPoseTracker() {}

VrpnPoseTracker::~VrpnPoseTracker() {
  tracker = nullptr;
}

void VrpnPoseTracker::setConnectionString(std::string id) {
  tracker = std::make_unique<vrpn_Tracker_Remote>(id.c_str());
  tracker->register_change_handler(this, VrpnPoseTracker::handler);
}

bool VrpnPoseTracker::getPose(std::map<int, PoseSample> &p) {

  if (!tracker) {
    GM_WRN("VrpnPoseTracker", "Cannot get pose - no vrpn connection");
    return false;
  }

  if (!tracker->connectionPtr()->doing_okay()) {
    GM_WRN("VrpnPoseTracker", "Defunct connection - closing vrpn connection");
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

void VRPN_CALLBACK VrpnPoseTracker::handler(void *data, const vrpn_TRACKERCB info) {
  VrpnPoseTracker *_this = static_cast<VrpnPoseTracker*>(data);

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
  GM_VINF("VrpnPoseTracker", "Got vrpn tracker data for sensor " << info.sensor);
}

END_NAMESPACE_GMTRACK;

#endif
