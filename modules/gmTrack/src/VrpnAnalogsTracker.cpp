
#include <gmTrack/VrpnAnalogsTracker.hh>

#ifdef gramods_ENABLE_VRPN

#include <gmCore/Console.hh>
#include <gmCore/RunOnce.hh>

#include <vrpn_Analog.h>

BEGIN_NAMESPACE_GMTRACK;

GM_OFI_DEFINE(VrpnAnalogsTracker);
GM_OFI_PARAM2(VrpnAnalogsTracker, connectionString, std::string, setConnectionString);

struct VrpnAnalogsTracker::Impl {

  void update();
  void setConnectionString(std::string id);
  bool getAnalogs(AnalogsSample &b);

	static void VRPN_CALLBACK handler(void *userdata, const vrpn_ANALOGCB info);
  void handler(const vrpn_ANALOGCB info);

  std::unique_ptr<vrpn_Analog_Remote> tracker;

  std::optional<AnalogsSample> latest_sample;
  bool got_data;
};

VrpnAnalogsTracker::VrpnAnalogsTracker()
  : Updateable(10), _impl(std::make_unique<Impl>()) {}

VrpnAnalogsTracker::~VrpnAnalogsTracker() {
  _impl->tracker = nullptr;
}

void VrpnAnalogsTracker::update(gmCore::Updateable::clock::time_point, size_t) {
  _impl->update();
}

void VrpnAnalogsTracker::Impl::update() {

  if (!tracker) {
    GM_RUNONCE(GM_WRN("VrpnAnalogsTracker", "Cannot get buttons - no vrpn connection"));
    return;
  }

  if (!tracker->connectionPtr()->doing_okay()) {
    GM_ERR("VrpnAnalogsTracker", "Defunct connection - closing vrpn connection");
    tracker = nullptr;
    latest_sample = std::nullopt;
    return;
  }

  do {
    got_data = false;
    tracker->mainloop();
  } while (got_data);
}

void VrpnAnalogsTracker::setConnectionString(std::string id) {
  _impl->setConnectionString(id);
}

void VrpnAnalogsTracker::Impl::setConnectionString(std::string id) {
  tracker = std::make_unique<vrpn_Analog_Remote>(id.c_str());
  tracker->register_change_handler(this, VrpnAnalogsTracker::Impl::handler);
}

bool VrpnAnalogsTracker::getAnalogs(AnalogsSample &b) {
  return _impl->getAnalogs(b);
}

bool VrpnAnalogsTracker::Impl::getAnalogs(AnalogsSample &b) {

  if (!latest_sample) return false;

  b = *latest_sample;

  return true;
}

void VRPN_CALLBACK VrpnAnalogsTracker::Impl::handler(void *data,
                                                     const vrpn_ANALOGCB info) {
  static_cast<VrpnAnalogsTracker::Impl*>(data)->handler(info);
}

void VRPN_CALLBACK VrpnAnalogsTracker::Impl::handler(const vrpn_ANALOGCB info) {

  typedef std::chrono::steady_clock clock;

  auto secs = std::chrono::duration_cast<clock::duration>
    (std::chrono::seconds(info.msg_time.tv_sec));
  auto usecs = std::chrono::duration_cast<clock::duration>
    (std::chrono::microseconds(info.msg_time.tv_usec));
  auto time = clock::time_point(secs + usecs);

  AnalogsSample sample;
  sample.time = time;
  sample.analogs.resize(info.num_channel, 0);

  std::stringstream analogs_log;
  for (size_t idx = 0; idx < (size_t)info.num_channel; ++idx) {
    sample.analogs[idx] = float(info.channel[idx]);
    analogs_log << info.channel[idx] << " ";
  }
  latest_sample = sample;

  got_data = true;
  GM_DBG3("VrpnAnalogsTracker", "Got vrpn analog data: " << analogs_log.str());
}

END_NAMESPACE_GMTRACK;

#endif
