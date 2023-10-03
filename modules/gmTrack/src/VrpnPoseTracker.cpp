
#include <gmTrack/VrpnPoseTracker.hh>

#ifdef gramods_ENABLE_VRPN

#include <gmCore/Console.hh>
#include <gmCore/RunOnce.hh>

#include <vrpn_Tracker.h>

BEGIN_NAMESPACE_GMTRACK;

GM_OFI_DEFINE(VrpnPoseTracker);
GM_OFI_PARAM2(VrpnPoseTracker, connectionString, std::string, setConnectionString);

struct VrpnPoseTracker::Impl {

  bool getPose(std::map<int, PoseSample> &p);
  void setConnectionString(std::string id);
  void update();

	static void VRPN_CALLBACK handler(void *userdata, const vrpn_TRACKERCB info);
  void handler(const vrpn_TRACKERCB info);

  std::unique_ptr<vrpn_Tracker_Remote> tracker;

  std::map<int, PoseSample> latest_samples;
  bool got_data;
};

VrpnPoseTracker::VrpnPoseTracker()
  : Updateable(10), _impl(std::make_unique<Impl>()) {}

VrpnPoseTracker::~VrpnPoseTracker() {
  _impl->tracker = nullptr;
}

void VrpnPoseTracker::update(gmCore::Updateable::clock::time_point, size_t) {
  _impl->update();
}

void VrpnPoseTracker::Impl::update() {

  if (!tracker) {
    GM_RUNONCE(GM_WRN("VrpnPoseTracker", "Cannot get pose - no vrpn connection"));
    return;
  }

  if (!tracker->connectionPtr()->doing_okay()) {
    GM_ERR("VrpnPoseTracker", "Defunct connection - closing vrpn connection");
    tracker = nullptr;
    latest_samples.clear();
    return;
  }

  do {
    got_data = false;
    tracker->mainloop();
  } while (got_data);
}

void VrpnPoseTracker::setConnectionString(std::string id) {
  _impl->setConnectionString(id);
}

void VrpnPoseTracker::Impl::setConnectionString(std::string id) {
  tracker = std::make_unique<vrpn_Tracker_Remote>(id.c_str());
  tracker->register_change_handler(this, VrpnPoseTracker::Impl::handler);
}

bool VrpnPoseTracker::getPose(std::map<int, PoseSample> &p) {
  return _impl->getPose(p);
}

bool VrpnPoseTracker::Impl::getPose(std::map<int, PoseSample> &p) {

  if (latest_samples.empty()) return false;

  p = latest_samples;

  return true;
}

void VRPN_CALLBACK VrpnPoseTracker::Impl::handler(void *data, const vrpn_TRACKERCB info) {
  static_cast<VrpnPoseTracker::Impl*>(data)->handler(info);
}

void VrpnPoseTracker::Impl::handler(const vrpn_TRACKERCB info) {

  typedef std::chrono::steady_clock clock;

  auto secs = std::chrono::duration_cast<clock::duration>
    (std::chrono::seconds(info.msg_time.tv_sec));
  auto usecs = std::chrono::duration_cast<clock::duration>
    (std::chrono::microseconds(info.msg_time.tv_usec));
  auto time = clock::time_point(secs + usecs);

  latest_samples[info.sensor].time = time;
  latest_samples[info.sensor].position =
      Eigen::Vector3d(info.pos[0], info.pos[1], info.pos[2]).cast<float>();
  latest_samples[info.sensor].orientation =
      Eigen::Quaterniond(info.quat[3], info.quat[0], info.quat[1], info.quat[2])
          .cast<float>();

  got_data = true;
  GM_DBG2("VrpnPoseTracker", "Got vrpn tracker data for sensor " << info.sensor);
}

END_NAMESPACE_GMTRACK;

#endif
