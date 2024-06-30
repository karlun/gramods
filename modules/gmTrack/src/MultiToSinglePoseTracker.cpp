
#include <gmTrack/MultiToSinglePoseTracker.hh>

#include <gmCore/Console.hh>
#include <gmCore/RunOnce.hh>


BEGIN_NAMESPACE_GMTRACK;

GM_OFI_DEFINE(MultiToSinglePoseTracker);
GM_OFI_POINTER2(MultiToSinglePoseTracker, multiPoseTracker, MultiPoseTracker, setMultiPoseTracker);
GM_OFI_PARAM2(MultiToSinglePoseTracker, sensor, int, setSensor);


bool MultiToSinglePoseTracker::getPose(PoseSample &p) {

  if (!tracker) {
    GM_RUNONCE(GM_WRN("MultiToSinglePoseTracker", "Pose requested but no pose tracker available."));
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

void MultiToSinglePoseTracker::traverse(Visitor *visitor) {
  if (tracker) tracker->accept(visitor);
}

END_NAMESPACE_GMTRACK;
