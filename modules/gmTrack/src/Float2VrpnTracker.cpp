
#include <gmCore/io_eigen.hh>

#include <gmTrack/Float2VrpnTracker.hh>
#include <gmTrack/VrpnTracker.impl.hh>

BEGIN_NAMESPACE_GMTRACK;

GM_OFI_DEFINE(Float2VrpnTracker);
GM_OFI_PARAM2(Float2VrpnTracker, connectionString, std::string, addConnectionString);

template<>
void VrpnTracker<gmCore::float2, vrpn_Analog_Remote, vrpn_ANALOGCB>::Impl::
    setState(State &state,
             clock::time_point now,
             const std::string &key,
             vrpn_ANALOGCB info) {

  std::stringstream log;
  if (info.num_channel == 2) [[likely]] {
    state[key] = {.time = now,
                  .value = {float(info.channel[0]), float(info.channel[1])}};
  } else {
    GM_RUNONCE(
        GM_WRN("Float2VrpnTracker",
               "Unexpected channel count (" << info.num_channel << " != 2)"));
    state[key] = {
        .time = now,
        .value = {float(info.channel[0]),
                  info.num_channel > 2 ? float(info.channel[1]) : 0.f}};
  }

  GM_DBG3(type_str,
          "Got data for " << key << ": " << state[key].value[0] << " "
                          << state[key].value[1]);
}

template class VrpnTracker<gmCore::float2, vrpn_Analog_Remote, vrpn_ANALOGCB>;

END_NAMESPACE_GMTRACK;
