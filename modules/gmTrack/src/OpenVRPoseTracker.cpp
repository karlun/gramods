
#include <gmTrack/OpenVRPoseTracker.hh>

#ifdef gramods_ENABLE_OpenVR

#include <gmCore/Console.hh>
#include <gmCore/RunOnce.hh>

#include <optional>

BEGIN_NAMESPACE_GMTRACK;

GM_OFI_DEFINE(OpenVRPoseTracker);
GM_OFI_PARAM2(OpenVRPoseTracker, type, std::string, setType);
GM_OFI_PARAM2(OpenVRPoseTracker, role, std::string, setRole);
GM_OFI_PARAM2(OpenVRPoseTracker, serial, std::string, setSerial);
GM_OFI_POINTER2(OpenVRPoseTracker, openVR, gmCore::OpenVR, setOpenVR);

struct OpenVRPoseTracker::Impl {
  bool getPose(PoseSample &p);
  bool checkParamFit(size_t idx);
  PoseSample extractPose(const vr::TrackedDevicePose_t &tdp);

  std::shared_ptr<gmCore::OpenVR> openvr;

  std::optional<vr::ETrackedDeviceClass> type;
  std::optional<vr::ETrackedControllerRole> role;
  std::optional<std::string> serial;

  std::optional<size_t> tracker_idx;
};

OpenVRPoseTracker::OpenVRPoseTracker() : _impl(new Impl()) {}
OpenVRPoseTracker::~OpenVRPoseTracker() {}

void OpenVRPoseTracker::setType(std::string type) {
  if (type.empty()) {
    _impl->type = std::nullopt;
    return;
  }

#define TYPE(CODE)                                                             \
  if (type == #CODE) {                                                         \
    _impl->type = vr::TrackedDeviceClass_##CODE;                               \
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

void OpenVRPoseTracker::setRole(std::string role) {
  if (role.empty()) {
    _impl->role = std::nullopt;
    return;
  }

#define ROLE(CODE)                                                             \
  if (role == #CODE) {                                                         \
    _impl->role = vr::TrackedControllerRole_##CODE;                            \
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

void OpenVRPoseTracker::setSerial(std::string s) {
  if (s.empty())
    _impl->serial = std::nullopt;
  else
    _impl->serial = s;
}

bool OpenVRPoseTracker::getPose(PoseSample &p) {
  return _impl->getPose(p);
}

bool OpenVRPoseTracker::Impl::checkParamFit(size_t idx) {

  if (!openvr) return false;

  auto dev_serial = openvr->getSerial(idx);
  auto dev_type = openvr->getType(idx);
  auto dev_role = openvr->getRole(idx);

  if (serial && dev_serial != *serial) {
    GM_DBG2("OpenVRPoseTracker", "checkParamFit fail for device " << idx << " against serial " << dev_serial << " != " << *serial);
    return false;
  }
  if (type && dev_type != *type) {
    GM_DBG2("OpenVRPoseTracker", "checkParamFit fail for device " << idx << " against type " << gmCore::OpenVR::typeToString(dev_type) << " != " << gmCore::OpenVR::typeToString(*type));
    return false;
  }
  if (role && dev_role != *role) {
    GM_DBG2("OpenVRPoseTracker", "checkParamFit fail for device " << idx << " against role " << gmCore::OpenVR::roleToString(dev_role) << " != " << gmCore::OpenVR::roleToString(*role));
    return false;
  }
  GM_DBG2("OpenVRPoseTracker", "checkParamFit match for device " << idx << ": " << dev_serial << ", " << gmCore::OpenVR::typeToString(dev_type) << ", " << gmCore::OpenVR::roleToString(dev_role));

  return true;
}

OpenVRPoseTracker::PoseSample
OpenVRPoseTracker::Impl::extractPose(const vr::TrackedDevicePose_t &tdp) {
  Eigen::Matrix4f M = gmCore::OpenVR::convert(tdp.mDeviceToAbsoluteTracking);
  Eigen::Quaternionf Q_rot;
  Q_rot = M.block<3, 3>(0, 0);
  return { M.block<3, 1>(0, 3), Q_rot, PoseTracker::clock::now() };
}

bool OpenVRPoseTracker::Impl::getPose(PoseSample &p) {

  if (!openvr) {
    GM_RUNONCE(GM_ERR("OpenVRPoseTracker",
                      "Cannot read pose data without OpenVR instance"));
    return false;
  }

  auto pose_list = openvr->getPoseList();
  if (!pose_list) return false;

  if (tracker_idx && (*pose_list)[*tracker_idx].bDeviceIsConnected &&
      checkParamFit(*tracker_idx)) {

    if (!(*pose_list)[*tracker_idx].bPoseIsValid) return false;

    p = extractPose((*pose_list)[*tracker_idx]);
    return true;
  }

  for (size_t idx = 0; idx < pose_list->size(); ++idx) {
    if (!(*pose_list)[idx].bDeviceIsConnected) continue;
    if (!(*pose_list)[idx].bPoseIsValid) continue;
    if (!checkParamFit(idx)) continue;

    tracker_idx = idx;
    p = extractPose((*pose_list)[idx]);
    return true;
  }

  tracker_idx = std::nullopt;
  return false;
}

void OpenVRPoseTracker::setOpenVR(std::shared_ptr<gmCore::OpenVR> openvr) {
  _impl->openvr = openvr;
}

void OpenVRPoseTracker::traverse(Visitor *visitor) {
  if (_impl->openvr) _impl->openvr->accept(visitor);
}

END_NAMESPACE_GMTRACK;

#endif
