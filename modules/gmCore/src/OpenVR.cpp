
#include <gmCore/OpenVR.hh>

#ifdef gramods_ENABLE_OpenVR

#include <gmCore/Console.hh>
#include <gmCore/RunOnce.hh>
#include <gmCore/FileResolver.hh>

#include <thread>
#include <optional>

BEGIN_NAMESPACE_GMCORE;

GM_OFI_DEFINE(OpenVR);
GM_OFI_PARAM2(OpenVR, useVideo, bool, setUseVideo);
GM_OFI_PARAM2(OpenVR, manifestPath, std::filesystem::path, setManifestPath);
GM_OFI_PARAM2(OpenVR, actionSet, std::string, setActionSet);

#define MAX_RETRY_COUNT 10
#define RETRY_DELAY_MS 2000

struct OpenVR::Impl : gmCore::Updateable {
  Impl();
  ~Impl();

  void update(clock::time_point time, size_t frame) override;
  void initialize();
  void setup_openvr();

  vr::IVRSystem *vr = nullptr;
  clock::time_point last_time = clock::time_point::max();

  std::optional<OpenVR::pose_list_t> &getPoseList();
  std::optional<OpenVR::pose_list_t> cache_poses;

  std::optional<vr::VRActionHandle_t> getHandle(std::string action);

  std::optional<vr::InputAnalogActionData_t>
  getAnalog(std::string action, vr::VRInputValueHandle_t device);
  std::optional<vr::InputDigitalActionData_t>
  getDigital(std::string action, vr::VRInputValueHandle_t device);
  std::unordered_map<std::string, vr::VRActionHandle_t> action_handles;

  std::string getSerial(size_t idx);
  std::unordered_map<size_t, std::string> cache_serial;

  static bool has_instance;
  bool use_video = false;

  std::optional<std::filesystem::path> manifest_path;
  std::optional<std::string> action_set;
  std::optional<vr::VRActionSetHandle_t> action_set_handle;

  std::vector<std::weak_ptr<EventConsumer>> event_consumers;
};

bool OpenVR::Impl::has_instance = false;

namespace {
std::string getEvrInputError(vr::EVRInputError err) {
#define ERROR(CODE)                                                          \
    if (err == vr::VRInputError_##CODE) { return #CODE; }
  ERROR(None);
  ERROR(NameNotFound);
  ERROR(WrongType);
  ERROR(InvalidHandle);
  ERROR(InvalidParam);
  ERROR(NoSteam);
  ERROR(MaxCapacityReached);
  ERROR(IPCError);
  ERROR(NoActiveActionSet);
  ERROR(InvalidDevice);
  ERROR(InvalidSkeleton);
  ERROR(InvalidBoneCount);
  ERROR(InvalidCompressedData);
  ERROR(NoData);
  ERROR(BufferTooSmall);
  ERROR(MismatchedActionManifest);
  ERROR(MissingSkeletonData);
  ERROR(InvalidBoneIndex);
  ERROR(InvalidPriority);
  ERROR(PermissionDenied);
  ERROR(InvalidRenderModel);
#undef ERROR
  throw gmCore::InvalidArgument(GM_STR("Unknown error code: " << err));
}
}

OpenVR::OpenVR() : _impl(new Impl()) {}
OpenVR::~OpenVR() {}

OpenVR::Impl::Impl() {
  if (has_instance)
    throw RuntimeException("Cannot have more than one instance of OpenVR");
  has_instance = true;
}
OpenVR::Impl::~Impl() {
  vr::VR_Shutdown();
  vr = nullptr;
  GM_DBG1("OpenVR", "De-initialized OpenVR");
  has_instance = false;
}

void OpenVR::setUseVideo(bool on) { _impl->use_video = on; }

void OpenVR::setManifestPath(std::filesystem::path path) {
  _impl->manifest_path = FileResolver::getDefault()->resolve(
      path, FileResolver::Check::ReadableFile);
}

void OpenVR::setActionSet(std::string name) { _impl->action_set = name; }

void OpenVR::initialize() {
  _impl->initialize();
  Object::initialize();
}

void OpenVR::Impl::initialize() {
  vr::EVRInitError error = vr::VRInitError_None;
  for (size_t try_count = 0; try_count < MAX_RETRY_COUNT; ++try_count) {
    vr = vr::VR_Init(&error,
                     use_video ? vr::VRApplication_Scene
                               : vr::VRApplication_Utility);
    if (error == vr::VRInitError_None) {
      GM_DBG1("OpenVR", "Successfully initialized OpenVR");
      setup_openvr();
      return;
    }

    GM_WRN("OpenVR",
           "Could not initialize OpenVR: "
               << vr::VR_GetVRInitErrorAsEnglishDescription(error)
               << ". Waiting 2 seconds then retrying.");
    std::this_thread::sleep_for(std::chrono::milliseconds(RETRY_DELAY_MS));
    GM_WRN("OpenVR", "Retry nr " << try_count);
  }

  throw RuntimeException(
      GM_STR("Could not initialize OpenVR: "
             << vr::VR_GetVRInitErrorAsEnglishDescription(error)));
}

void OpenVR::Impl::setup_openvr() {

  if (!manifest_path) { GM_WRN("OpenVR", "Missing path to manifest file."); }
  if (!action_set) { GM_WRN("OpenVR", "Missing action set."); }

  if (!manifest_path || !action_set) return;

  auto err =
        vr::VRInput()->SetActionManifestPath(manifest_path->u8string().c_str());
  if (err != vr::VRInputError_None) 
    GM_WRN("OpenVR",
           "Failed to set manifest path to " << *manifest_path << ": "
                                             << getEvrInputError(err));
}

std::optional<OpenVR::pose_list_t> &OpenVR::getPoseList() {
  return _impl->getPoseList();
}

std::optional<OpenVR::pose_list_t> &OpenVR::Impl::getPoseList() {
  static std::optional<OpenVR::pose_list_t> no_list = std::nullopt;

  if (cache_poses) return cache_poses;
  if (!vr) return no_list;

  cache_poses = OpenVR::pose_list_t();
  if (use_video)
    vr::VRCompositor()->WaitGetPoses(
        cache_poses->data(), vr::k_unMaxTrackedDeviceCount, NULL, 0);
  else
    vr->GetDeviceToAbsoluteTrackingPose(vr::TrackingUniverseRawAndUncalibrated,
                                        0,
                                        cache_poses->data(),
                                        vr::k_unMaxTrackedDeviceCount);

  return cache_poses;
}

void OpenVR::addEventConsumer(std::weak_ptr<EventConsumer> ec) {
  if (!ec.lock()) return;
  _impl->event_consumers.push_back(ec);
}

void OpenVR::removeEventConsumer(EventConsumer *ec) {
  if (!ec) return;
  auto &v = _impl->event_consumers;
  v.erase(std::remove_if(v.begin(),
                         v.end(),
                         [ec](const std::weak_ptr<EventConsumer> &ptr) {
                           return ptr.lock().get() == ec;
                         }),
          v.end());
}

Updateable::clock::time_point OpenVR::getLastTime() {
  return _impl->last_time;
}

std::optional<vr::InputAnalogActionData_t>
OpenVR::getAnalog(std::string action, vr::VRInputValueHandle_t device) {
  return _impl->getAnalog(action, device);
}

std::optional<vr::InputAnalogActionData_t>
OpenVR::Impl::getAnalog(std::string action, vr::VRInputValueHandle_t device) {
  auto handle = getHandle(action);
  if (!handle) return std::nullopt;

  vr::InputAnalogActionData_t data;
  auto err =
      vr::VRInput()->GetAnalogActionData(*handle, &data, sizeof(data), device);
  if (err != vr::VRInputError_None) {
    GM_RUNONCE(
        GM_ERR("OpenVR",
               "Failed to get analog action data: " << getEvrInputError(err)));
    return std::nullopt;
  }

  return data;
}

std::optional<vr::InputDigitalActionData_t>
OpenVR::getDigital(std::string action, vr::VRInputValueHandle_t device) {
  return _impl->getDigital(action, device);
}

std::optional<vr::InputDigitalActionData_t>
OpenVR::Impl::getDigital(std::string action, vr::VRInputValueHandle_t device) {

  auto handle = getHandle(action);
  if (!handle) return std::nullopt;

  vr::InputDigitalActionData_t data;
  auto err =
      vr::VRInput()->GetDigitalActionData(*handle, &data, sizeof(data), device);
  if (err != vr::VRInputError_None) {
    GM_ERR("OpenVR",
           "Failed to get analog action data: " << getEvrInputError(err));
    return std::nullopt;
  }

  return data;
}

std::optional<vr::VRActionHandle_t>
OpenVR::Impl::getHandle(std::string action) {

  if (action_handles.count(action) > 0) return action_handles[action];

  vr::VRActionHandle_t handle;
  auto err = vr::VRInput()->GetActionHandle(action.c_str(), &handle);

  if (err != vr::VRInputError_None) {
    GM_RUNONCE(GM_ERR("OpenVR", "Failed to get action handle: " << getEvrInputError(err)));
    return std::nullopt;
  }

  action_handles[action] = handle;
  return handle;
}

void OpenVR::Impl::update(clock::time_point time, size_t frame) {
  cache_poses = std::nullopt;
  cache_serial.clear();

  vr::VREvent_t event;
  while (vr->PollNextEvent(&event, sizeof(event))) {

    bool consumed = false;
    for (auto w_ptr : event_consumers) {
      auto s_ptr = w_ptr.lock();
      consumed |= s_ptr->processEvent(&event, consumed);
    }

    GM_DBG1("OpenVR",
            "Event " << event.eventType << " ("
                     << vr->GetEventTypeNameFromEnum(
                            vr::EVREventType(event.eventType))
                     << ") on device " << event.trackedDeviceIndex
                     << (consumed ? " consumed" : " not consumed"));
  }

  last_time = clock::now();

  if (!action_set) return;

  if (!action_set_handle) {
    vr::VRActionSetHandle_t handle;
    auto err = vr::VRInput()->GetActionSetHandle(action_set->c_str(), &handle);
    if (err == vr::VRInputError_None) {
      action_set_handle = handle;
    } else {
      GM_RUNONCE(
          GM_ERR("OpenVR",
                 "Could not retrieve handle for action set " << *action_set));
    }
  }

  if (!action_set_handle) return;

  vr::VRActiveActionSet_t actionSet = {0};
  actionSet.ulActionSet = *action_set_handle;
  vr::VRInput()->UpdateActionState(&actionSet, sizeof(actionSet), 1);
}

vr::IVRSystem *OpenVR::ptr() { return _impl->vr; }

vr::ETrackedDeviceClass OpenVR::getType(size_t idx) {
  if (!_impl->vr) return vr::TrackedDeviceClass_Invalid;
  return _impl->vr->GetTrackedDeviceClass(idx);
}

vr::ETrackedControllerRole OpenVR::getRole(size_t idx) {
  if (!_impl->vr) return vr::TrackedControllerRole_Invalid;
  return _impl->vr->GetControllerRoleForTrackedDeviceIndex(idx);
}

std::string OpenVR::getSerial(size_t idx) {
  return _impl->getSerial(idx);
}

std::string OpenVR::Impl::getSerial(size_t idx) {
  if (cache_serial.count(idx) > 0) return cache_serial[idx];

  vr::ETrackedPropertyError err;
  std::vector<char> serial_buffer(512);
  uint32_t len = vr::VRSystem()->GetStringTrackedDeviceProperty(
      idx, vr::Prop_SerialNumber_String, serial_buffer.data(), 512, &err);

  if (err != vr::TrackedProp_Success) {
    GM_ERR(
        "OpenVR",
        "While reading tracker " << idx << " serial: " << vr->GetPropErrorNameFromEnum(err));
    return "";
  }

  if (len == 0) return "";

  std::string serial(serial_buffer.begin(), serial_buffer.begin() + len - 1);

  cache_serial[idx] = serial;
  return serial;
}

Eigen::Matrix4f OpenVR::convert(const vr::HmdMatrix34_t &m) {
  Eigen::Matrix4f M;
  M << m.m[0][0], m.m[0][1], m.m[0][2], m.m[0][3],
       m.m[1][0], m.m[1][1], m.m[1][2], m.m[1][3],
       m.m[2][0], m.m[2][1], m.m[2][2], m.m[2][3],
       0.f, 0.f, 0.f, 1.f;
  return M;
}

std::string OpenVR::typeToString(vr::ETrackedDeviceClass type) {

#define TYPE(CODE)                                                \
    if (type == vr::TrackedDeviceClass_##CODE) { return #CODE; }
  TYPE(Invalid);
  TYPE(HMD);
  TYPE(Controller);
  TYPE(GenericTracker);
  TYPE(TrackingReference);
  TYPE(DisplayRedirect);
#undef TYPE

  return "Unknown";
}

std::string OpenVR::roleToString(vr::ETrackedControllerRole role) {

#define ROLE(CODE)                                              \
    if (role == vr::TrackedControllerRole_##CODE) return #CODE;
  ROLE(Invalid);
  ROLE(LeftHand);
  ROLE(RightHand);
  ROLE(OptOut);
  ROLE(Treadmill);
  ROLE(Stylus);
#undef ROLE

  return "Unknown";
}

END_NAMESPACE_GMCORE;

#endif
