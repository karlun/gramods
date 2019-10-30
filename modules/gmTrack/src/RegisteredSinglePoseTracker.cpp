

#include <gmTrack/RegisteredSinglePoseTracker.hh>

#include <gmTypes/eigen.hh>

BEGIN_NAMESPACE_GMTRACK;

GM_OFI_DEFINE(RegisteredSinglePoseTracker);
GM_OFI_POINTER(RegisteredSinglePoseTracker, poseTracker, SinglePoseTracker, RegisteredSinglePoseTracker::setPoseTracker);
GM_OFI_PARAM(RegisteredSinglePoseTracker, registrationMatrix, Eigen::Matrix4f, RegisteredSinglePoseTracker::setRegistrationMatrix);


bool RegisteredSinglePoseTracker::getPose(PoseSample &p) {

  if (!tracker) return false;

  PoseSample pose;
  if (!tracker->getPose(pose))
    return false;

  PoseSample reg_pose;
  reg_pose.time = pose.time;

  reg_pose.position =
    (registration * (pose.position + position_bias).homogeneous())
    .block<3,1>(0,0);
  reg_pose.orientation = Eigen::Quaternionf(registration.block<3,3>(0,0)) *
    (pose.orientation * orientation_bias);

  p = reg_pose;

  return true;
}

END_NAMESPACE_GMTRACK;
