

#include <gmTrack/RegisteredSinglePoseTracker.hh>

#include <gmCore/Console.hh>
#include <gmCore/RunOnce.hh>

#include <gmCore/io_eigen.hh>

BEGIN_NAMESPACE_GMTRACK;

GM_OFI_DEFINE(RegisteredSinglePoseTracker);
GM_OFI_POINTER2(RegisteredSinglePoseTracker, singlePoseTracker, SinglePoseTracker, setSinglePoseTracker);
GM_OFI_PARAM2(RegisteredSinglePoseTracker, registrationMatrix, Eigen::Matrix4f, setRegistrationMatrix);
GM_OFI_PARAM2(RegisteredSinglePoseTracker, biasMatrix, Eigen::Matrix4f, setBiasMatrix);
GM_OFI_PARAM2(RegisteredSinglePoseTracker, positionBias, Eigen::Vector3f, setPositionBias);
GM_OFI_PARAM2(RegisteredSinglePoseTracker, orientationBias, Eigen::Quaternionf, setOrientationBias);

struct RegisteredSinglePoseTracker::Impl {

  bool getPose(PoseSample &p);

  std::shared_ptr<SinglePoseTracker> tracker;

  Eigen::Matrix4f reg_matrix = Eigen::Matrix4f::Identity();
  Eigen::Matrix4f bias_matrix = Eigen::Matrix4f::Identity();
  Eigen::Quaternionf reg_rotation = Eigen::Quaternionf::Identity();
  Eigen::Quaternionf bias_rotation = Eigen::Quaternionf::Identity();
};

RegisteredSinglePoseTracker::RegisteredSinglePoseTracker()
  : _impl(std::make_unique<Impl>()) {}
RegisteredSinglePoseTracker::~RegisteredSinglePoseTracker() {}

void RegisteredSinglePoseTracker::setSinglePoseTracker(std::shared_ptr<SinglePoseTracker> tracker) {
  _impl->tracker = tracker;
}

void RegisteredSinglePoseTracker::setRegistrationMatrix(Eigen::Matrix4f m) {
  _impl->reg_matrix = m;
  _impl->reg_rotation = Eigen::Quaternionf(m.block<3, 3>(0, 0)).normalized();
}

void RegisteredSinglePoseTracker::setBiasMatrix(Eigen::Matrix4f m) {
  _impl->bias_matrix = m;
  _impl->bias_rotation = Eigen::Quaternionf(m.block<3, 3>(0, 0)).normalized();
}

void RegisteredSinglePoseTracker::setPositionBias(Eigen::Vector3f p) {
  _impl->bias_matrix.block<3,1>(0,3) = p;
}

void RegisteredSinglePoseTracker::setOrientationBias(Eigen::Quaternionf q) {
  _impl->bias_matrix.block<3,3>(0,0) = Eigen::Matrix3f(q);
  _impl->bias_rotation = q;
}

bool RegisteredSinglePoseTracker::getPose(PoseSample &p) {
  return _impl->getPose(p);
}

bool RegisteredSinglePoseTracker::Impl::getPose(PoseSample &p) {

  if (!tracker) {
    GM_RUNONCE(GM_WRN("RegisteredSinglePoseTracker", "Pose requested but no pose tracker available."));
    return false;
  }

  PoseSample raw_pose;
  if (!tracker->getPose(raw_pose))
    return false;

  PoseSample reg_pose;
  reg_pose.time = raw_pose.time;

  const Eigen::Affine3f M = raw_pose.asMatrix();
  reg_pose.position = (reg_matrix * M.matrix() * bias_matrix).block<3, 1>(0, 3);
  reg_pose.orientation = reg_rotation * raw_pose.orientation * bias_rotation;

  p = reg_pose;

  return true;
}

void RegisteredSinglePoseTracker::traverse(Visitor *visitor) {
  if (_impl->tracker) _impl->tracker->accept(visitor);
}

END_NAMESPACE_GMTRACK;
