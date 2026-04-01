

#include <gmTrack/OffsetPoseTracker.hh>

#include <gmCore/Console.hh>
#include <gmCore/RunOnce.hh>

#include <gmCore/io_eigen.hh>

#include <unordered_set>

BEGIN_NAMESPACE_GMTRACK;

GM_OFI_DEFINE(OffsetPoseTracker);
GM_OFI_POINTER2(OffsetPoseTracker, poseTracker, PoseTracker, setPoseTracker);
GM_OFI_PARAM2(OffsetPoseTracker, positionOffset, Eigen::Vector3f, setPositionOffset);
GM_OFI_PARAM2(OffsetPoseTracker, orientationOffset, Eigen::Quaternionf, setOrientationOffset);
GM_OFI_PARAM2(OffsetPoseTracker, offsetMatrix, Eigen::Matrix4f, setOffsetMatrix);

struct OffsetPoseTracker::Impl {

  std::optional<State> get();

  std::shared_ptr<PoseTracker> tracker;

  std::unordered_set<std::string> keys;

  Eigen::Vector3f position_offset = Eigen::Vector3f::Zero();
  Eigen::Quaternionf orientation_offset = Eigen::Quaternionf::Identity();
};

OffsetPoseTracker::OffsetPoseTracker() : _impl(std::make_unique<Impl>()) {}
OffsetPoseTracker::~OffsetPoseTracker() {}

void OffsetPoseTracker::setPoseTracker(std::shared_ptr<PoseTracker> tracker) {
  _impl->tracker = tracker;
}

void OffsetPoseTracker::addKey(std::string key) {
  _impl->keys.insert(key);
}

std::optional<PoseTracker::State> OffsetPoseTracker::get() { return _impl->get(); }

std::optional<PoseTracker::State> OffsetPoseTracker::Impl::get() {

  if (!tracker) {
    GM_RUNONCE(GM_WRN("OffsetPoseTracker", "Pose requested but no pose tracker available."));
    return std::nullopt;
  }

  auto raw_state = tracker->get();
  if (!raw_state) return std::nullopt;

  State reg_state;

  for (const auto as : raw_state.value())
    if (keys.contains(as.first)) {
      reg_state[as.first] = {
          .time = as.second.time,
          .value = {.position = as.second.value.position +
                                as.second.value.orientation * position_offset,
                    .orientation =
                        as.second.value.orientation * orientation_offset}};
    } else {
      reg_state[as.first] = as.second;
    }

  return reg_state;
}

void OffsetPoseTracker::setPositionOffset(Eigen::Vector3f p) {
  _impl->position_offset = p;
}

void OffsetPoseTracker::setOrientationOffset(Eigen::Quaternionf q) {
  _impl->orientation_offset = q;
}

void OffsetPoseTracker::setOffsetMatrix(Eigen::Matrix4f m) {

  _impl->position_offset = m.block<3,1>(0,3);

  Eigen::JacobiSVD<Eigen::MatrixXf> svd(m.block(0,0,3,3),
                                        Eigen::ComputeFullU | Eigen::ComputeFullV);
  auto U = svd.matrixU();
  auto S = svd.singularValues();
  auto V = svd.matrixV();

  Eigen::Matrix3f R = U * V.transpose();
  _impl->orientation_offset = Eigen::Quaternionf(R);
}

void OffsetPoseTracker::traverse(Visitor *visitor) {
  if (_impl->tracker) _impl->tracker->accept(visitor);
}

END_NAMESPACE_GMTRACK;
