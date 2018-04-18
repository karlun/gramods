
#include <gmTrack/VRPNTracker.hh>


#ifdef GM_ENABLE_VRPN

BEGIN_NAMESPACE_GMTRACK;

VRPNTracker::VRPNTracker() {
  setUpdateRate(30);
}

VRPNTracker::~VRPNTracker() {
  tracker = nullptr;
  thread.join();
}

void VRPNTracker::setConnectionString(std::string id) {
  std::lock_guard<std::mutex> guard(lock);

  tracker = nullptr;
  thread.join();

  tracker = std::make_unique<vrpn_Tracker_Remote>(id.c_str());
  tracker->register_change_handler(this, VRPNTracker::handler);
  thread = std::thread(&VRPNTracker::_update, this);
}

void VRPNTracker::setUpdateRate(double fps) {
  frame_time = std::chrono::duration_cast<clock::duration>
    (std::chrono::duration<double>(1.0 / fps));
  cond.notify_one();
}

bool VRPNTracker::getPose(std::map<int, PoseSample> &p) {
  std::lock_guard<std::mutex> guard(lock);
  p = latest_samples;
}

void VRPNTracker::_update() {

  std::mutex mutex;
  std::unique_lock<std::mutex> lock(mutex);

  clock::time_point last_time = clock::now();
  while (tracker) {

    tracker->mainloop();

    while (true) {
      clock::time_point current_time = clock::now();
      if (current_time >= last_time + frame_time)
        break;
      cond.wait_for(lock, frame_time - (current_time - last_time));
    }
    last_time = last_time + frame_time;
  }
}

void VRPN_CALLBACK VRPNTracker::handler(void *data, const vrpn_TRACKERCB info) {
  VRPNTracker *_this = static_cast<VRPNTracker*>(data);

  auto secs = std::chrono::duration_cast<clock::duration>
    (std::chrono::seconds(info.msg_time.tv_sec));
  auto usecs = std::chrono::duration_cast<clock::duration>
    (std::chrono::microseconds(info.msg_time.tv_usec));

  {
    std::lock_guard<std::mutex> guard(_this->lock);

    _this->latest_samples[info.sensor].time =
      clock::time_point(secs + usecs);
    _this->latest_samples[info.sensor].position =
      Eigen::Vector3f(info.pos[0], info.pos[1], info.pos[2]);
    _this->latest_samples[info.sensor].orientation =
      Eigen::Quaternionf(info.quat[0], info.quat[1], info.quat[2], info.quat[3]);
  }
}

END_NAMESPACE_GMTRACK;

#endif
