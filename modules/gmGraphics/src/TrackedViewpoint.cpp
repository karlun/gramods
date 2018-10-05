
#include <gmGraphics/TrackedViewpoint.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE_SUB(TrackedViewpoint, Viewpoint);
GM_OFI_POINTER(TrackedViewpoint, tracker, gmTrack::SinglePoseTracker, TrackedViewpoint::setTracker);

struct TrackedViewpoint::Impl {

  void getPosition(Eigen::Vector3f &p);
  void getOrientation(Eigen::Quaternionf &q);

  std::shared_ptr<gmTrack::SinglePoseTracker> tracker;
};

TrackedViewpoint::TrackedViewpoint()
  : _impl(std::make_unique<Impl>()) {}

Eigen::Vector3f TrackedViewpoint::getPosition() {
  _impl->getPosition(position);
  return position;
}

Eigen::Quaternionf TrackedViewpoint::getOrientation() {
  _impl->getOrientation(orientation);
  return orientation;
}

void TrackedViewpoint::Impl::getPosition(Eigen::Vector3f &p) {

  if (!tracker) return;

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

void TrackedViewpoint::setTracker(std::shared_ptr<gmTrack::SinglePoseTracker> t) {
  _impl->tracker = t;
}

END_NAMESPACE_GMGRAPHICS;
