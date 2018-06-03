
#include <gmTrack/RelativeSinglePoseTracker.hh>

BEGIN_NAMESPACE_GMTRACK;


GM_OFI_DEFINE(RelativeSinglePoseTracker);
GM_OFI_POINTER(RelativeSinglePoseTracker, originTracker,
               SinglePoseTracker, RelativeSinglePoseTracker::setOriginTracker);
GM_OFI_POINTER(RelativeSinglePoseTracker, targetTracker,
               SinglePoseTracker, RelativeSinglePoseTracker::setTargetTracker);


bool RelativeSinglePoseTracker::getPose(PoseSample &p) {
  if (!origin_tracker || !target_tracker) return false;

  PoseSample origin_sample;
  if (!origin_tracker->getPose(origin_sample))
    return false;

  PoseSample target_sample;
  if (!target_tracker->getPose(target_sample))
    return false;

  if (origin_sample.time > target_sample.time)
    p = origin_sample;
  else
    p = target_sample;

  p.orientation =
    origin_sample.orientation.conjugate()
    * target_sample.orientation;
  p.position =
    origin_sample.orientation.conjugate()
    * (target_sample.position - origin_sample.position);

  return true;
}

void RelativeSinglePoseTracker::setOriginTracker(std::shared_ptr<SinglePoseTracker> tracker) {
  origin_tracker = tracker;
}

void RelativeSinglePoseTracker::setTargetTracker(std::shared_ptr<SinglePoseTracker> tracker) {
  target_tracker = tracker;
}

END_NAMESPACE_GMTRACK;

