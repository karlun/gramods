
#include <gmCore/io_eigen.hh>

#include <gmTrack/BinaryVrpnTracker.hh>
#include <gmTrack/VrpnTracker.impl.hh>

BEGIN_NAMESPACE_GMTRACK;

GM_OFI_DEFINE(BinaryVrpnTracker);
GM_OFI_PARAM2(BinaryVrpnTracker, connectionString, std::string, addConnectionString);

template<>
void VrpnTracker<bool, vrpn_Button_Remote, vrpn_BUTTONCB>::Impl::setState(
    State &state,
    clock::time_point now,
    const std::string &key,
    vrpn_BUTTONCB info) {

  const auto name = GM_STR(key << "/" << info.button);
  state[name] = {.time = now, .value = info.state != 0};

  GM_DBG3(type_str, "Got data for " << key << ", button " << info.button);
}

template class VrpnTracker<bool, vrpn_Button_Remote, vrpn_BUTTONCB>;

END_NAMESPACE_GMTRACK;
