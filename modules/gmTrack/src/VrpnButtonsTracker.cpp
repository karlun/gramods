
#include <gmTrack/VrpnButtonsTracker.hh>

#ifdef gramods_ENABLE_VRPN

#include <gmCore/Console.hh>
#include <gmCore/RunOnce.hh>

#include <vrpn_Button.h>

BEGIN_NAMESPACE_GMTRACK;

GM_OFI_DEFINE(VrpnButtonsTracker);
GM_OFI_PARAM2(VrpnButtonsTracker, connectionString, std::string, setConnectionString);

struct VrpnButtonsTracker::Impl {

  void update();
  void setConnectionString(std::string id);
  bool getButtons(ButtonsSample &p);

  static void VRPN_CALLBACK handler(void *data, const vrpn_BUTTONCB info);
  void handler(const vrpn_BUTTONCB info);

  std::unique_ptr<vrpn_Button_Remote> tracker;

  std::optional<ButtonsSample> latest_sample;
  bool got_data;
};

VrpnButtonsTracker::VrpnButtonsTracker()
  : Updateable(10), _impl(std::make_unique<Impl>()) {}

VrpnButtonsTracker::~VrpnButtonsTracker() {
  _impl->tracker = nullptr;
}

void VrpnButtonsTracker::update(gmCore::Updateable::clock::time_point, size_t) {
  _impl->update();
}

void VrpnButtonsTracker::Impl::update() {

  if (!tracker) {
    GM_RUNONCE(GM_WRN("VrpnButtonsTracker", "Cannot get buttons - no vrpn connection"));
    return;
  }

  if (!tracker->connectionPtr()->doing_okay()) {
    GM_ERR("VrpnButtonsTracker", "Defunct connection - closing vrpn connection");
    tracker = nullptr;
    latest_sample = std::nullopt;
    return;
  }

  do {
    got_data = false;
    tracker->mainloop();
  } while (got_data);
}

void VrpnButtonsTracker::setConnectionString(std::string id) {
  _impl->setConnectionString(id);
}

void VrpnButtonsTracker::Impl::setConnectionString(std::string id) {
  tracker = std::make_unique<vrpn_Button_Remote>(id.c_str());
  tracker->register_change_handler(this, handler);
}

bool VrpnButtonsTracker::getButtons(ButtonsSample &p) {
  return _impl->getButtons(p);
}

bool VrpnButtonsTracker::Impl::getButtons(ButtonsSample &p) {

  if (!latest_sample) return false;

  p = *latest_sample;

  return true;
}

void VRPN_CALLBACK VrpnButtonsTracker::Impl::handler(void *data,
                                                     const vrpn_BUTTONCB info) {
  static_cast<VrpnButtonsTracker::Impl*>(data)->handler(info);
}

void VrpnButtonsTracker::Impl::handler(const vrpn_BUTTONCB info) {

  typedef std::chrono::steady_clock clock;

  auto secs = std::chrono::duration_cast<clock::duration>
    (std::chrono::seconds(info.msg_time.tv_sec));
  auto usecs = std::chrono::duration_cast<clock::duration>
    (std::chrono::microseconds(info.msg_time.tv_usec));
  auto time = clock::time_point(secs + usecs);

  if (!latest_sample) latest_sample = ButtonsSample {};
  latest_sample->time = time;
  if (info.state)
    latest_sample->buttons[info.button] = true;
  else
    latest_sample->buttons[info.button] = false;

  got_data = true;

  GM_DBG3("VrpnButtonsTracker", "Got vrpn button data for button " << info.button << " (" << info.state << ")");
}

END_NAMESPACE_GMTRACK;

#endif
