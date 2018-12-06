
#include <gmTrack/VRPNAnalogsTracker.hh>

#ifdef gramods_ENABLE_VRPN

#include <gmCore/Console.hh>

BEGIN_NAMESPACE_GMTRACK;

GM_OFI_DEFINE(VRPNAnalogsTracker);
GM_OFI_PARAM(VRPNAnalogsTracker, connectionString, std::string, VRPNAnalogsTracker::setConnectionString);

VRPNAnalogsTracker::VRPNAnalogsTracker() {}

VRPNAnalogsTracker::~VRPNAnalogsTracker() {
  tracker = nullptr;
}

void VRPNAnalogsTracker::setConnectionString(std::string id) {
  tracker = std::make_unique<vrpn_Analog_Remote>(id.c_str());
  tracker->register_change_handler(this, VRPNAnalogsTracker::handler);
}

bool VRPNAnalogsTracker::getAnalogs(AnalogsSample &b) {

  if (!tracker) {
    GM_WRN("VRPNAnalogsTracker", "Cannot get buttons - no vrpn connection");
    return false;
  }

  if (!tracker->connectionPtr()->doing_okay()) {
    GM_WRN("VRPNAnalogsTracker", "Defunct connection - closing vrpn connection");
    tracker = nullptr;
    return false;
  }

  do {
    got_data = false;
    tracker->mainloop();
  } while (got_data);

  b = latest_sample;

  return true;
}

void VRPN_CALLBACK VRPNAnalogsTracker::handler(void *data, const vrpn_ANALOGCB info) {
  VRPNAnalogsTracker *_this = static_cast<VRPNAnalogsTracker*>(data);

  auto secs = std::chrono::duration_cast<clock::duration>
    (std::chrono::seconds(info.msg_time.tv_sec));
  auto usecs = std::chrono::duration_cast<clock::duration>
    (std::chrono::microseconds(info.msg_time.tv_usec));
  auto time = clock::time_point(secs + usecs);

  _this->latest_sample.time = time;
  _this->latest_sample.analogs.resize(info.num_channel, 0);

  std::stringstream analogs_log;
  for (size_t idx = 0; idx < info.num_channel; ++idx) {
    _this->latest_sample.analogs[idx] = info.channel[idx];
    analogs_log << info.channel[idx] << " ";
  }

  _this->got_data = true;
  GM_VVINF("VRPNAnalogsTracker", "Got vrpn analog data: " << analogs_log.str());
}

END_NAMESPACE_GMTRACK;

#endif
