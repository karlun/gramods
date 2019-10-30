

#include <gmTrack/OffsetSinglePoseTracker.hh>

#include <gmCore/Console.hh>
#include <gmCore/RunOnce.hh>

#include <gmTypes/eigen.hh>

BEGIN_NAMESPACE_GMTRACK;

GM_OFI_DEFINE(OffsetSinglePoseTracker);
GM_OFI_POINTER(OffsetSinglePoseTracker, poseTracker, SinglePoseTracker, OffsetSinglePoseTracker::setPoseTracker);
GM_OFI_PARAM(OffsetSinglePoseTracker, positionOffset, Eigen::Vector3f, OffsetSinglePoseTracker::setPositionOffset);
GM_OFI_PARAM(OffsetSinglePoseTracker, orientationOffset, Eigen::Quaternionf, OffsetSinglePoseTracker::setOrientationOffset);


bool OffsetSinglePoseTracker::getPose(PoseSample &p) {

  if (!tracker) {
    GM_RUNONCE(GM_WRN("OffsetSinglePoseTracker", "Pose requested by no pose tracker available."));
    return false;
  }

  PoseSample pose;
  if (!tracker->getPose(pose))
    return false;

  p.time = pose.time;

  p.position = pose.position + pose.orientation * position_offset;
  p.orientation = pose.orientation * orientation_offset;

  return true;
}

END_NAMESPACE_GMTRACK;
