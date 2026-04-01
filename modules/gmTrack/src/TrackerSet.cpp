
#include <gmTrack/TrackerSet.hh>

#include <gmCore/Console.hh>
#include <gmCore/RunOnce.hh>

BEGIN_NAMESPACE_GMTRACK;

GM_OFI_DEFINE(TrackerSet);
GM_OFI_POINTER2(TrackerSet, poseTracker, PoseTracker, setPoseTracker);
GM_OFI_POINTER2(TrackerSet, binaryTracker, BinaryTracker, setBinaryTracker);
GM_OFI_POINTER2(TrackerSet, floatTracker, FloatTracker, setFloatTracker);
GM_OFI_POINTER2(TrackerSet, float2Tracker, Float2Tracker, setFloat2Tracker);


std::optional<PoseTracker::State> TrackerSet::getPose() {
  if (!pose_tracker) {
    GM_RUNONCE(GM_WRN("TrackerSet", "PoseSample requested but no pose tracker available."));
    return std::nullopt;
  }
  return pose_tracker->get();
}

std::optional<BinaryTracker::State> TrackerSet::getBinary() {
  if (!binary_tracker) {
    GM_RUNONCE(GM_WRN("TrackerSet", "BinarySample requested but no binary tracker available."));
    return std::nullopt;
  }
  return binary_tracker->get();
}

std::optional<FloatTracker::State> TrackerSet::getFloat() {
  if (!float_tracker) {
    GM_RUNONCE(GM_WRN("TrackerSet", "FloatSample requested but no float tracker available."));
    return std::nullopt;
  }
  return float_tracker->get();
}

std::optional<Float2Tracker::State> TrackerSet::getFloat2() {
  if (!float2_tracker) {
    GM_RUNONCE(GM_WRN("TrackerSet", "Float2Sample requested but no float2 tracker available."));
    return std::nullopt;
  }
  return float2_tracker->get();
}

void TrackerSet::setPoseTracker(std::shared_ptr<PoseTracker> t) {
  pose_tracker = t;
}

void TrackerSet::setBinaryTracker(std::shared_ptr<BinaryTracker> t) {
  binary_tracker = t;
}

void TrackerSet::setFloatTracker(std::shared_ptr<FloatTracker> t) {
  float_tracker = t;
}

void TrackerSet::setFloat2Tracker(std::shared_ptr<Float2Tracker> t) {
  float2_tracker = t;
}

void TrackerSet::traverse(Visitor *visitor) {
  if (pose_tracker) pose_tracker->accept(visitor);
  if (binary_tracker) binary_tracker->accept(visitor);
  if (float_tracker) float_tracker->accept(visitor);
  if (float2_tracker) float2_tracker->accept(visitor);
}

END_NAMESPACE_GMTRACK;
