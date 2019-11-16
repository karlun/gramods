
#include <gmTrack/MultiToSinglePoseTracker.hh>

#include <gmCore/Console.hh>
#include <gmCore/RunOnce.hh>


BEGIN_NAMESPACE_GMTRACK;

GM_OFI_DEFINE(MultiToSinglePoseTracker);
GM_OFI_POINTER(MultiToSinglePoseTracker, multiPoseTracker, MultiPoseTracker, MultiToSinglePoseTracker::setMultiPoseTracker);
GM_OFI_PARAM(MultiToSinglePoseTracker, sensor, int, MultiToSinglePoseTracker::setSensor);


bool MultiToSinglePoseTracker::getPose(PoseSample &p) {

  if (!tracker) {
    GM_RUNONCE(GM_WRN("MultiToSinglePoseTracker", "Pose requested by no pose tracker available."));
    return false;
  }

  std::map<int, PoseSample> pose;
  if (!tracker->getPose(pose))
    return false;

  if (pose.count(sensor_idx) == 0)
    return false;

  p = pose[sensor_idx];

  return true;
}

END_NAMESPACE_GMTRACK;
