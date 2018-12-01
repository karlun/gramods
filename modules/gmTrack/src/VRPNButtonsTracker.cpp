
#include <gmTrack/VRPNButtonsTracker.hh>

#ifdef gramods_ENABLE_VRPN

#include <gmCore/Console.hh>

BEGIN_NAMESPACE_GMTRACK;

GM_OFI_DEFINE(VRPNButtonsTracker);
GM_OFI_PARAM(VRPNButtonsTracker, connectionString, std::string, VRPNButtonsTracker::setConnectionString);

VRPNButtonsTracker::VRPNButtonsTracker() {}

VRPNButtonsTracker::~VRPNButtonsTracker() {
  tracker = nullptr;
}

void VRPNButtonsTracker::setConnectionString(std::string id) {
  tracker = std::make_unique<vrpn_Button_Remote>(id.c_str());
  tracker->register_change_handler(this, VRPNButtonsTracker::handler);
}

bool VRPNButtonsTracker::getButtons(ButtonsSample &p) {

  if (!tracker) {
    GM_WRN("VRPNButtonsTracker", "Cannot get buttons - no vrpn connection");
    return false;
  }

  if (!tracker->connectionPtr()->doing_okay()) {
    GM_WRN("VRPNButtonsTracker", "Defunct connection - closing vrpn connection");
    tracker = nullptr;
    return false;
  }

  do {
    got_data = false;
    tracker->mainloop();
  } while (got_data);

  p = latest_sample;

  return true;
}

void VRPN_CALLBACK VRPNButtonsTracker::handler(void *data, const vrpn_BUTTONCB info) {
  VRPNButtonsTracker *_this = static_cast<VRPNButtonsTracker*>(data);

  auto secs = std::chrono::duration_cast<clock::duration>
    (std::chrono::seconds(info.msg_time.tv_sec));
  auto usecs = std::chrono::duration_cast<clock::duration>
    (std::chrono::microseconds(info.msg_time.tv_usec));
  auto time = clock::time_point(secs + usecs);

  if (info.button >= 32) return;

  _this->latest_sample.time = time;
  if (info.state)
    _this->latest_sample.buttons |= 0x1 << info.button;
  else
    _this->latest_sample.buttons &= ~(0x1 << info.button);

  if (info.button == 0)
    _this->latest_sample.main_button = (0x1 & info.button) != 0;

  _this->got_data = true;
  GM_VVINF("VRPNButtonsTracker", "Got vrpn button data for button " << info.button);
}

END_NAMESPACE_GMTRACK;

#endif
