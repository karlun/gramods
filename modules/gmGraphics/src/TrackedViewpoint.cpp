
#include <gmGraphics/TrackedViewpoint.hh>

#include <gmCore/Console.hh>
#include <gmCore/RunOnce.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE_SUB(TrackedViewpoint, Viewpoint);
GM_OFI_POINTER2(TrackedViewpoint, singlePoseTracker, gmTrack::SinglePoseTracker, setSinglePoseTracker);

struct TrackedViewpoint::Impl {

  void getPosition(Eigen::Vector3f &p);
  void getOrientation(Eigen::Quaternionf &q);

  std::shared_ptr<gmTrack::SinglePoseTracker> tracker;
};

TrackedViewpoint::TrackedViewpoint()
  : _impl(std::make_unique<Impl>()) {}

Eigen::Vector3f TrackedViewpoint::getPosition(Eye eye) {
  _impl->getPosition(position);
  return Viewpoint::getPosition(eye);
}

Eigen::Quaternionf TrackedViewpoint::getOrientation(Eye eye) {
  _impl->getOrientation(orientation);
  return Viewpoint::getOrientation(eye);
}

void TrackedViewpoint::Impl::getPosition(Eigen::Vector3f &p) {

  if (!tracker) {
    GM_RUNONCE(GM_WRN("TrackedViewpoint", "Tracker requested but no tracker available."));
    return;
  }

  gmTrack::PoseTracker::PoseSample pose;
  if (tracker->getPose(pose))
    p = pose.position;
}

void TrackedViewpoint::Impl::getOrientation(Eigen::Quaternionf &q) {

  if (!tracker) return;

  gmTrack::PoseTracker::PoseSample pose;
  if (tracker->getPose(pose))
    q = pose.orientation;
}

void TrackedViewpoint::setSinglePoseTracker(std::shared_ptr<gmTrack::SinglePoseTracker> t) {
  _impl->tracker = t;
}

void TrackedViewpoint::traverse(Visitor *visitor) {
  if (_impl->tracker) _impl->tracker->accept(visitor);
}

END_NAMESPACE_GMGRAPHICS;
