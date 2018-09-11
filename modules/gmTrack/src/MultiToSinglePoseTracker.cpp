
#include <gmTrack/MultiToSinglePoseTracker.hh>

BEGIN_NAMESPACE_GMTRACK;

GM_OFI_DEFINE(MultiToSinglePoseTracker);
GM_OFI_POINTER(MultiToSinglePoseTracker, tracker, MultiPoseTracker, MultiToSinglePoseTracker::setTracker);
GM_OFI_PARAM(MultiToSinglePoseTracker, sensor, int, MultiToSinglePoseTracker::setSensor);


bool MultiToSinglePoseTracker::getPose(PoseSample &p) {

  if (!tracker) return false;

  std::map<int, PoseSample> pose;
  if (!tracker->getPose(pose))
    return false;

  if (pose.count(sensor_idx) == 0)
    return false;

  p = pose[sensor_idx];

  return true;
}

END_NAMESPACE_GMTRACK;