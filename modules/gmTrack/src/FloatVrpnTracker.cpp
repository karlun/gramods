
#include <gmCore/io_eigen.hh>

#include <gmTrack/FloatVrpnTracker.hh>
#include <gmTrack/VrpnTracker.impl.hh>

#ifdef gramods_ENABLE_VRPN

BEGIN_NAMESPACE_GMTRACK;

GM_OFI_DEFINE(FloatVrpnTracker);
GM_OFI_PARAM2(FloatVrpnTracker, connectionString, std::string, addConnectionString);

template<>
void VrpnTracker<float, vrpn_Analog_Remote, vrpn_ANALOGCB>::Impl::setState(
    State &state,
    clock::time_point now,
    const std::string &key,
    vrpn_ANALOGCB info) {

  std::stringstream log;
  if (info.num_channel == 1) {
    state[key] = {.time = now, .value = float(info.channel[0])};
  } else {
    for (vrpn_int32 idx = 0; idx < info.num_channel; ++idx) {
      const auto name = GM_STR(key << "/" << idx);
      state[name] = {.time = now, .value = float(info.channel[idx])};
      log << idx << "=" << float(info.channel[idx]) << " ";
    }
  }

  std::string log_str = log.str();
  log_str.pop_back();

  GM_DBG3(type_str, "Got data for " << key << ": " << log_str);
}

template class VrpnTracker<float, vrpn_Analog_Remote, vrpn_ANALOGCB>;

END_NAMESPACE_GMTRACK;

#endif
