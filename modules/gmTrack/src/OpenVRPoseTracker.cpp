
#include <gmTrack/OpenVRPoseTracker.hh>

#ifdef gramods_ENABLE_OpenVR

#include <gmCore/Console.hh>
#include <gmCore/RunOnce.hh>

#include <optional>
#include <unordered_set>

BEGIN_NAMESPACE_GMTRACK;

GM_OFI_DEFINE(OpenVRPoseTracker);
GM_OFI_PARAM2(OpenVRPoseTracker, type, std::string, addType);
GM_OFI_PARAM2(OpenVRPoseTracker, role, std::string, addRole);
GM_OFI_PARAM2(OpenVRPoseTracker, serial, std::string, addSerial);
GM_OFI_POINTER2(OpenVRPoseTracker, openVR, gmCore::OpenVR, setOpenVR);

struct OpenVRPoseTracker::Impl {
  std::optional<State> get();
  bool checkParamFit(size_t idx);
  std::vector<std::string> createKeysFromParam(size_t idx);
  gmCore::Pose extractPose(const vr::TrackedDevicePose_t &tdp);

  std::shared_ptr<gmCore::OpenVR> openvr;

  std::unordered_set<vr::ETrackedDeviceClass> type;
  std::unordered_set<vr::ETrackedControllerRole> role;
  std::unordered_set<std::string> serial;
};

OpenVRPoseTracker::OpenVRPoseTracker() : _impl(new Impl()) {}
OpenVRPoseTracker::~OpenVRPoseTracker() {}

void OpenVRPoseTracker::addType(std::string type) {
#define TYPE(CODE)                                                             \
  if (type == #CODE) {                                                         \
    _impl->type.insert(vr::TrackedDeviceClass_##CODE);                         \
    return;                                                                    \
  }

  TYPE(HMD);
  TYPE(Controller);
  TYPE(GenericTracker);
  TYPE(TrackingReference);
  TYPE(DisplayRedirect);

  throw gmCore::InvalidArgument(GM_STR(
      "Unknown device type (class): "
      << type << "! "
      << "Must be one of HMD, Controller, GenericTracker, TrackingReference, DisplayRedirect"));

#undef TYPE
}

void OpenVRPoseTracker::addRole(std::string role) {
#define ROLE(CODE)                                                             \
  if (role == #CODE) {                                                         \
    _impl->role.insert(vr::TrackedControllerRole_##CODE);                      \
    return;                                                                    \
  }

  ROLE(LeftHand);
  ROLE(RightHand);
  ROLE(OptOut);
  ROLE(Treadmill);
  ROLE(Stylus);

  throw gmCore::InvalidArgument(GM_STR(
      "Unknown device role: "
      << role << "! "
      << "Must be one of LeftHand, RightHand, OptOut, Treadmill, Stylus"));

#undef ROLE
}

void OpenVRPoseTracker::addSerial(std::string s) { _impl->serial.insert(s); }

std::optional<PoseTracker::State> OpenVRPoseTracker::get() {
  return _impl->get();
}

bool OpenVRPoseTracker::Impl::checkParamFit(size_t idx) {

  if (!openvr) return false;

  auto dev_serial = openvr->getSerial(idx);
  auto dev_type = openvr->getType(idx);
  auto dev_role = openvr->getRole(idx);

  if (!serial.empty() && serial.contains(dev_serial)) {
    GM_DBG2("OpenVRPoseTracker",
            "checkParamFit fail for device " << idx << ", serial "
                                             << dev_serial);
    return false;
  }
  if (!type.empty() && type.contains(dev_type)) {
    GM_DBG2("OpenVRPoseTracker",
            "checkParamFit fail for device "
                << idx << ", type " << gmCore::OpenVR::typeToString(dev_type));
    return false;
  }
  if (!role.empty() && role.contains(dev_role)) {
    GM_DBG2("OpenVRPoseTracker",
            "checkParamFit fail for device "
                << idx << ", role " << gmCore::OpenVR::roleToString(dev_role));
    return false;
  }
  GM_DBG2("OpenVRPoseTracker",
          "checkParamFit match for device "
              << idx << ": " << dev_serial << ", "
              << gmCore::OpenVR::typeToString(dev_type) << ", "
              << gmCore::OpenVR::roleToString(dev_role));

  return true;
}

std::vector<std::string> OpenVRPoseTracker::Impl::createKeysFromParam(size_t idx) {
  const auto dev_serial = openvr->getSerial(idx);
  const auto dev_type = gmCore::OpenVR::typeToString(openvr->getType(idx));
  const auto dev_role = gmCore::OpenVR::roleToString(openvr->getRole(idx));
  return {GM_STR("/in/pose/serial/" << dev_serial),
          GM_STR("/in/pose/type/" << dev_type),
          GM_STR("/in/pose/role/" << dev_role),
          GM_STR("/in/pose/" << dev_type << "/" << dev_role)};
}


gmCore::Pose
OpenVRPoseTracker::Impl::extractPose(const vr::TrackedDevicePose_t &tdp) {
  Eigen::Matrix4f M = gmCore::OpenVR::convert(tdp.mDeviceToAbsoluteTracking);
  Eigen::Quaternionf Q_rot;
  Q_rot = M.block<3, 3>(0, 0);
  return {.position = M.block<3, 1>(0, 3), .orientation = Q_rot};
}

std::optional<PoseTracker::State> OpenVRPoseTracker::Impl::get() {

  if (!openvr) {
    GM_RUNONCE(GM_ERR("OpenVRPoseTracker",
                      "Cannot read pose data without OpenVR instance"));
    return std::nullopt;
  }

  auto pose_list = openvr->getPoseList();
  if (!pose_list) return std::nullopt;

  State state;
  const auto now = clock::now();

  for (size_t idx = 0; idx < pose_list->size(); ++idx) {
    if (!(*pose_list)[idx].bDeviceIsConnected) continue;
    if (!(*pose_list)[idx].bPoseIsValid) continue;
    if (!checkParamFit(idx)) continue;

    const auto pose = extractPose((*pose_list)[idx]);
    for (const auto key : createKeysFromParam(idx))
      state[key] = {.time = now, .value = pose};
  }

  if (state.empty()) return std::nullopt;
  return state;
}

void OpenVRPoseTracker::setOpenVR(std::shared_ptr<gmCore::OpenVR> openvr) {
  _impl->openvr = openvr;
}

void OpenVRPoseTracker::traverse(Visitor *visitor) {
  if (_impl->openvr) _impl->openvr->accept(visitor);
}

END_NAMESPACE_GMTRACK;

#endif
