

#include <gmTrack/RegisteredMultiPoseTracker.hh>

#include <gmCore/Console.hh>
#include <gmCore/RunOnce.hh>

#include <gmCore/eigen.hh>

BEGIN_NAMESPACE_GMTRACK;

GM_OFI_DEFINE(RegisteredMultiPoseTracker);
GM_OFI_POINTER2(RegisteredMultiPoseTracker, multiPoseTracker, MultiPoseTracker, setMultiPoseTracker);
GM_OFI_PARAM2(RegisteredMultiPoseTracker, registrationMatrix, Eigen::Matrix4f, setRegistrationMatrix);
GM_OFI_PARAM2(RegisteredMultiPoseTracker, positionBias, Eigen::Vector3f, setPositionBias);
GM_OFI_PARAM2(RegisteredMultiPoseTracker, orientationBias, Eigen::Quaternionf, setOrientationBias);


bool RegisteredMultiPoseTracker::getPose(std::map<int, PoseSample> &p) {

  if (!tracker) {
    GM_RUNONCE(GM_WRN("RegisteredMultiPoseTracker", "Pose requested but no pose tracker available."));
    return false;
  }

  std::map<int, PoseSample> poses;
  if (!tracker->getPose(poses))
    return false;

  for (auto pose : poses) {

    PoseSample reg_pose;
    reg_pose.time = pose.second.time;

    reg_pose.position =
      (registration * (pose.second.position + position_bias).homogeneous())
      .block<3,1>(0,0);
    reg_pose.orientation = Eigen::Quaternionf(registration.block<3,3>(0,0)) *
      (pose.second.orientation * orientation_bias);

    p[pose.first] = reg_pose;
  }

  return true;
}

END_NAMESPACE_GMTRACK;
