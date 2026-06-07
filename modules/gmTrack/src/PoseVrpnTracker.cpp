
#include <gmCore/io_eigen.hh>

#include <gmTrack/PoseVrpnTracker.hh>
#include <gmTrack/VrpnTracker.impl.hh>

#ifdef gramods_ENABLE_VRPN

BEGIN_NAMESPACE_GMTRACK;

GM_OFI_DEFINE(PoseVrpnTracker);
GM_OFI_PARAM2(PoseVrpnTracker, connectionString, std::string, addConnectionString);

template<>
void VrpnTracker<gmCore::Pose, vrpn_Tracker_Remote, vrpn_TRACKERCB>::Impl::
    setState(State &state,
             clock::time_point now,
             const std::string &key,
             vrpn_TRACKERCB info) {

  const auto name = GM_STR(key << "/" << info.sensor);
  state[name] = {
      .time = now,
      .value = gmCore::Pose {
          .position = Eigen::Vector3d(info.pos[0], info.pos[1], info.pos[2])
                          .cast<float>(),
          .orientation =
              Eigen::Quaterniond(
                  info.quat[3], info.quat[0], info.quat[1], info.quat[2])
                  .cast<float>()}};

  GM_DBG3(type_str, "Got data for " << key << ", sensor " << info.sensor);
}

template class VrpnTracker<gmCore::Pose, vrpn_Tracker_Remote, vrpn_TRACKERCB>;

END_NAMESPACE_GMTRACK;

#endif
