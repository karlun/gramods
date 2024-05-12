
#include <gmTrack/RelativeSinglePoseTracker.hh>

#include <gmCore/RunOnce.hh>
#include <gmCore/Console.hh>

BEGIN_NAMESPACE_GMTRACK;

GM_OFI_DEFINE(RelativeSinglePoseTracker);
GM_OFI_POINTER2(RelativeSinglePoseTracker, originTracker, SinglePoseTracker, setOriginTracker);
GM_OFI_POINTER2(RelativeSinglePoseTracker, targetTracker, SinglePoseTracker, setTargetTracker);

bool RelativeSinglePoseTracker::getPose(PoseSample &p) {

  if (!target_tracker) {
    GM_RUNONCE(GM_WRN("RelativeSinglePoseTracker", "Pose requested but no target tracker available."));
    return false;
  }

  if (!origin_tracker) {
    GM_RUNONCE(GM_WRN("RelativeSinglePoseTracker", "Pose requested but no origin tracker available."));
    return false;
  }

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

void RelativeSinglePoseTracker::traverse(Visitor *visitor) {
  if (origin_tracker) origin_tracker->accept(visitor);
  if (target_tracker) target_tracker->accept(visitor);
}

END_NAMESPACE_GMTRACK;

