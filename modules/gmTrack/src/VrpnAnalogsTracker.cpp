
#include <gmTrack/VrpnAnalogsTracker.hh>

#ifdef gramods_ENABLE_VRPN

#include <gmCore/Console.hh>

BEGIN_NAMESPACE_GMTRACK;

GM_OFI_DEFINE(VrpnAnalogsTracker);
GM_OFI_PARAM(VrpnAnalogsTracker, connectionString, std::string, VrpnAnalogsTracker::setConnectionString);

VrpnAnalogsTracker::VrpnAnalogsTracker()
  : Updateable(10) {}

VrpnAnalogsTracker::~VrpnAnalogsTracker() {
  tracker = nullptr;
}

void VrpnAnalogsTracker::update(gmCore::Updateable::clock::time_point t) {

  if (!tracker) {
    GM_WRN("VrpnAnalogsTracker", "Cannot get buttons - no vrpn connection");
    return;
  }

  if (!tracker->connectionPtr()->doing_okay()) {
    GM_WRN("VrpnAnalogsTracker", "Defunct connection - closing vrpn connection");
    tracker = nullptr;
    have_data = false;
    return;
  }

  do {
    got_data = false;
    tracker->mainloop();
  } while (got_data);
}

void VrpnAnalogsTracker::setConnectionString(std::string id) {
  tracker = std::make_unique<vrpn_Analog_Remote>(id.c_str());
  tracker->register_change_handler(this, VrpnAnalogsTracker::handler);
}

bool VrpnAnalogsTracker::getAnalogs(AnalogsSample &b) {

  if (!have_data)
    return false;

  b = latest_sample;

  return true;
}

void VRPN_CALLBACK VrpnAnalogsTracker::handler(void *data, const vrpn_ANALOGCB info) {
  VrpnAnalogsTracker *_this = static_cast<VrpnAnalogsTracker*>(data);

  typedef std::chrono::steady_clock clock;

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
  _this->have_data = true;
  GM_VVINF("VrpnAnalogsTracker", "Got vrpn analog data: " << analogs_log.str());
}

END_NAMESPACE_GMTRACK;

#endif
