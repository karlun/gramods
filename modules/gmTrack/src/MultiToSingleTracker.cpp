
#include <gmTrack/MultiToSingleTracker.hh>

BEGIN_NAMESPACE_GMTRACK;

GM_OFI_DEFINE(MultiToSingleTracker);
GM_OFI_POINTER(MultiToSingleTracker, tracker, MultiTracker, MultiToSingleTracker::setTracker);
GM_OFI_PARAM(MultiToSingleTracker, sensor, int, MultiToSingleTracker::setSensor);


bool MultiToSingleTracker::getPose(PoseSample &p) {

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
