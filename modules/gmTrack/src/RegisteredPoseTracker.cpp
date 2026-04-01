

#include <gmTrack/RegisteredPoseTracker.hh>

#include <gmCore/Console.hh>
#include <gmCore/RunOnce.hh>

#include <gmCore/io_eigen.hh>

#include <unordered_set>

BEGIN_NAMESPACE_GMTRACK;

GM_OFI_DEFINE(RegisteredPoseTracker);
GM_OFI_POINTER2(RegisteredPoseTracker, poseTracker, PoseTracker, setPoseTracker);
GM_OFI_PARAM2(RegisteredPoseTracker, key, std::string, addKey);
GM_OFI_PARAM2(RegisteredPoseTracker, registrationMatrix, Eigen::Matrix4f, setRegistrationMatrix);
GM_OFI_PARAM2(RegisteredPoseTracker, biasMatrix, Eigen::Matrix4f, setBiasMatrix);
GM_OFI_PARAM2(RegisteredPoseTracker, positionBias, Eigen::Vector3f, setPositionBias);
GM_OFI_PARAM2(RegisteredPoseTracker, orientationBias, Eigen::Quaternionf, setOrientationBias);

struct RegisteredPoseTracker::Impl {

  std::optional<State> get();

  std::shared_ptr<PoseTracker> tracker;

  std::unordered_set<std::string> keys;

  Eigen::Matrix4f reg_matrix = Eigen::Matrix4f::Identity();
  Eigen::Matrix4f bias_matrix = Eigen::Matrix4f::Identity();
  Eigen::Quaternionf reg_rotation = Eigen::Quaternionf::Identity();
  Eigen::Quaternionf bias_rotation = Eigen::Quaternionf::Identity();
};

RegisteredPoseTracker::RegisteredPoseTracker()
  : _impl(std::make_unique<Impl>()) {}
RegisteredPoseTracker::~RegisteredPoseTracker() {}

void RegisteredPoseTracker::setPoseTracker(std::shared_ptr<PoseTracker> tracker) {
  _impl->tracker = tracker;
}

void RegisteredPoseTracker::addKey(std::string key) {
  _impl->keys.insert(key);
}

void RegisteredPoseTracker::setRegistrationMatrix(Eigen::Matrix4f m) {
  _impl->reg_matrix = m;
  _impl->reg_rotation = Eigen::Quaternionf(m.block<3, 3>(0, 0)).normalized();
}

void RegisteredPoseTracker::setBiasMatrix(Eigen::Matrix4f m) {
  _impl->bias_matrix = m;
  _impl->bias_rotation = Eigen::Quaternionf(m.block<3, 3>(0, 0)).normalized();
}

void RegisteredPoseTracker::setPositionBias(Eigen::Vector3f p) {
  _impl->bias_matrix.block<3,1>(0,3) = p;
}

void RegisteredPoseTracker::setOrientationBias(Eigen::Quaternionf q) {
  _impl->bias_matrix.block<3,3>(0,0) = Eigen::Matrix3f(q);
  _impl->bias_rotation = q;
}

std::optional<PoseTracker::State> RegisteredPoseTracker::get() { return _impl->get(); }

std::optional<PoseTracker::State> RegisteredPoseTracker::Impl::get() {

  if (!tracker) {
    GM_RUNONCE(GM_WRN("RegisteredPoseTracker", "Pose requested but no pose tracker available."));
    return std::nullopt;
  }

  auto raw_state = tracker->get();
  if (!raw_state) return std::nullopt;

  State reg_state;

  if (keys.empty()) {
    for (const auto as : raw_state.value()) {
      const Eigen::Affine3f M = as.second.value.asMatrix();
      reg_state[as.first] = {
          .time = as.second.time,
          .value = {
              .position =
                  (reg_matrix * M.matrix() * bias_matrix).block<3, 1>(0, 3),
              .orientation =
                  reg_rotation * as.second.value.orientation * bias_rotation}};
    }
  } else {
    for (const auto as : raw_state.value())
      if (!keys.contains(as.first)) {
        reg_state[as.first] = as.second;
      } else {
        const Eigen::Affine3f M = as.second.value.asMatrix();
        reg_state[as.first] = {
            .time = as.second.time,
            .value = {
                .position =
                    (reg_matrix * M.matrix() * bias_matrix).block<3, 1>(0, 3),
                .orientation = reg_rotation * as.second.value.orientation *
                               bias_rotation}};
      }
  }

  return reg_state;
}

void RegisteredPoseTracker::traverse(Visitor *visitor) {
  if (_impl->tracker) _impl->tracker->accept(visitor);
}

END_NAMESPACE_GMTRACK;
