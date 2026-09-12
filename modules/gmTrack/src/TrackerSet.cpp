
#include <gmTrack/TrackerSet.hh>

#include <gmCore/Console.hh>
#include <gmCore/RunOnce.hh>

BEGIN_NAMESPACE_GMTRACK;

GM_OFI_DEFINE(TrackerSet);
GM_OFI_POINTER2(TrackerSet, poseTracker, PoseTracker, addPoseTracker);
GM_OFI_POINTER2(TrackerSet, binaryTracker, BinaryTracker, addBinaryTracker);
GM_OFI_POINTER2(TrackerSet, floatTracker, FloatTracker, addFloatTracker);
GM_OFI_POINTER2(TrackerSet, float2Tracker, Float2Tracker, addFloat2Tracker);


std::optional<PoseTracker::State> TrackerSet::getPose() {
  if (pose_tracker.empty()) {
    GM_RUNONCE(GM_WRN("TrackerSet",
                      "PoseSample requested but tracker not available."));
    return std::nullopt;
  }

  std::optional<PoseTracker::State> acc;
  for (const auto &tracker : pose_tracker)
    PoseTracker::merge(acc, tracker->get());

  return acc;
}

std::optional<BinaryTracker::State> TrackerSet::getBinary() {
  if (binary_tracker.empty()) {
    GM_RUNONCE(GM_WRN("TrackerSet",
                      "BinarySample requested but tracker not available."));
    return std::nullopt;
  }

  std::optional<BinaryTracker::State> acc;
  for (const auto &tracker : binary_tracker)
    BinaryTracker::merge(acc, tracker->get());

  return acc;
}

std::optional<FloatTracker::State> TrackerSet::getFloat() {
  if (float_tracker.empty()) {
    GM_RUNONCE(GM_WRN("TrackerSet",
                      "FloatSample requested but tracker not available."));
    return std::nullopt;
  }

  std::optional<FloatTracker::State> acc;
  for (const auto &tracker : float_tracker)
    FloatTracker::merge(acc, tracker->get());

  return acc;
}

std::optional<Float2Tracker::State> TrackerSet::getFloat2() {
  if (float2_tracker.empty()) {
    GM_RUNONCE(GM_WRN("TrackerSet",
                      "Float2Sample requested but tracker not available."));
    return std::nullopt;
  }

  std::optional<Float2Tracker::State> acc;
  for (const auto &tracker : float2_tracker)
    Float2Tracker::merge(acc, tracker->get());

  return acc;
}

void TrackerSet::addPoseTracker(std::shared_ptr<PoseTracker> t) {
  pose_tracker.push_back(t);
}

void TrackerSet::addBinaryTracker(std::shared_ptr<BinaryTracker> t) {
  binary_tracker.push_back(t);
}

void TrackerSet::addFloatTracker(std::shared_ptr<FloatTracker> t) {
  float_tracker.push_back(t);
}

void TrackerSet::addFloat2Tracker(std::shared_ptr<Float2Tracker> t) {
  float2_tracker.push_back(t);
}

void TrackerSet::traverse(Visitor *visitor) {
  for (auto &t : binary_tracker) t->accept(visitor);
  for (auto &t : float_tracker) t->accept(visitor);
  for (auto &t : float2_tracker) t->accept(visitor);
  for (auto &t : pose_tracker) t->accept(visitor);
}

END_NAMESPACE_GMTRACK;
