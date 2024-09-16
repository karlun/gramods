

#include <gmTrack/RegisteredMultiPoseTracker.hh>

#include <gmCore/Console.hh>
#include <gmCore/RunOnce.hh>

#include <gmCore/io_eigen.hh>

BEGIN_NAMESPACE_GMTRACK;

GM_OFI_DEFINE(RegisteredMultiPoseTracker);
GM_OFI_POINTER2(RegisteredMultiPoseTracker, singlePoseTracker, MultiPoseTracker, setMultiPoseTracker);
GM_OFI_PARAM2(RegisteredMultiPoseTracker, registrationMatrix, Eigen::Matrix4f, setRegistrationMatrix);
GM_OFI_PARAM2(RegisteredMultiPoseTracker, biasMatrix, Eigen::Matrix4f, setBiasMatrix);
GM_OFI_PARAM2(RegisteredMultiPoseTracker, positionBias, Eigen::Vector3f, setPositionBias);
GM_OFI_PARAM2(RegisteredMultiPoseTracker, orientationBias, Eigen::Quaternionf, setOrientationBias);

struct RegisteredMultiPoseTracker::Impl {

  bool getPose(std::map<int, PoseSample> &p);

  std::shared_ptr<MultiPoseTracker> tracker;

  Eigen::Matrix4f reg_matrix = Eigen::Matrix4f::Identity();
  Eigen::Matrix4f bias_matrix = Eigen::Matrix4f::Identity();
  Eigen::Quaternionf reg_rotation = Eigen::Quaternionf::Identity();
  Eigen::Quaternionf bias_rotation = Eigen::Quaternionf::Identity();
};

RegisteredMultiPoseTracker::RegisteredMultiPoseTracker()
  : _impl(std::make_unique<Impl>()) {}
RegisteredMultiPoseTracker::~RegisteredMultiPoseTracker() {}

void RegisteredMultiPoseTracker::setMultiPoseTracker(
    std::shared_ptr<MultiPoseTracker> tracker) {
  _impl->tracker = tracker;
}

void RegisteredMultiPoseTracker::setRegistrationMatrix(Eigen::Matrix4f m) {
  _impl->reg_matrix = m;
  _impl->reg_rotation = Eigen::Quaternionf(m.block<3, 3>(0, 0));
}

void RegisteredMultiPoseTracker::setBiasMatrix(Eigen::Matrix4f m) {
  _impl->bias_matrix = m;
  _impl->bias_rotation = Eigen::Quaternionf(m.block<3, 3>(0, 0));
}

void RegisteredMultiPoseTracker::setPositionBias(Eigen::Vector3f p) {
  _impl->bias_matrix.block<3,1>(0,3) = p;
}

void RegisteredMultiPoseTracker::setOrientationBias(Eigen::Quaternionf q) {
  _impl->bias_matrix.block<3,3>(0,0) = Eigen::Matrix3f(q);
  _impl->bias_rotation = q;
}

bool RegisteredMultiPoseTracker::getPose(std::map<int, PoseSample> &p) {
  return _impl->getPose(p);
}

bool RegisteredMultiPoseTracker::Impl::getPose(std::map<int, PoseSample> &p) {

  if (!tracker) {
    GM_RUNONCE(GM_WRN("RegisteredMultiPoseTracker", "Pose requested but no pose tracker available."));
    return false;
  }

  std::map<int, PoseSample> poses;
  if (!tracker->getPose(poses))
    return false;

  for (auto raw_pose : poses) {

    PoseSample reg_pose;
    reg_pose.time = raw_pose.second.time;

    const Eigen::Affine3f M = raw_pose.second.asMatrix();
    reg_pose.position =
        (reg_matrix * M.matrix() * bias_matrix).block<3, 1>(0, 3);
    reg_pose.orientation =
        reg_rotation * raw_pose.second.orientation * bias_rotation;

    p[raw_pose.first] = reg_pose;
  }

  return true;
}

void RegisteredMultiPoseTracker::traverse(Visitor *visitor) {
  if (_impl->tracker) _impl->tracker->accept(visitor);
}

END_NAMESPACE_GMTRACK;
