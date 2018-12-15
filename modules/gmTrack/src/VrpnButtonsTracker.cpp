
#include <gmTrack/VrpnButtonsTracker.hh>

#ifdef gramods_ENABLE_VRPN

#include <gmCore/Console.hh>

BEGIN_NAMESPACE_GMTRACK;

GM_OFI_DEFINE(VrpnButtonsTracker);
GM_OFI_PARAM(VrpnButtonsTracker, connectionString, std::string, VrpnButtonsTracker::setConnectionString);

VrpnButtonsTracker::VrpnButtonsTracker() {}

VrpnButtonsTracker::~VrpnButtonsTracker() {
  tracker = nullptr;
}

void VrpnButtonsTracker::setConnectionString(std::string id) {
  tracker = std::make_unique<vrpn_Button_Remote>(id.c_str());
  tracker->register_change_handler(this, VrpnButtonsTracker::handler);
}

bool VrpnButtonsTracker::getButtons(ButtonsSample &p) {

  if (!tracker) {
    GM_WRN("VrpnButtonsTracker", "Cannot get buttons - no vrpn connection");
    return false;
  }

  if (!tracker->connectionPtr()->doing_okay()) {
    GM_WRN("VrpnButtonsTracker", "Defunct connection - closing vrpn connection");
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

void VRPN_CALLBACK VrpnButtonsTracker::handler(void *data, const vrpn_BUTTONCB info) {
  VrpnButtonsTracker *_this = static_cast<VrpnButtonsTracker*>(data);

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

  _this->got_data = true;
  GM_VVINF("VrpnButtonsTracker", "Got vrpn button data for button " << info.button << " (" << info.state << ")");
}

END_NAMESPACE_GMTRACK;

#endif
