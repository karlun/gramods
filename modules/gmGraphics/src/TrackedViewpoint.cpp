
#include <gmGraphics/TrackedViewpoint.hh>

#include <gmCore/Console.hh>
#include <gmCore/RunOnce.hh>
#include <gmTrack/StdKey.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE_SUB(TrackedViewpoint, Viewpoint);
GM_OFI_PARAM2(TrackedViewpoint, key, std::string, setKey);
GM_OFI_POINTER2(TrackedViewpoint, poseTracker, gmTrack::PoseTracker, setPoseTracker);

struct TrackedViewpoint::Impl {

  void getPosition(Eigen::Vector3f &p);
  void getOrientation(Eigen::Quaternionf &q);

  std::shared_ptr<gmTrack::PoseTracker> tracker;

  std::optional<std::string> key;
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
    GM_RUNONCE(GM_WRN("TrackedViewpoint",
                      "Tracker requested but no tracker available."));
    return;
  }

  auto state = tracker->get();
  if (!state) {
    GM_RUNONCE(GM_WRN("TrackedViewpoint", //
                      "State requested but no state returned."));
    return;
  }

  auto key = this->key.value_or(gmTrack::StdKey::HEAD_POSE);
  if (!state->contains(key)) {
    GM_RUNONCE(GM_WRN("TrackedViewpoint", //
                      "Key " << key << " requested but not found in state."));
    return;
  }

  p = state.value()[key].value.position;
}

void TrackedViewpoint::Impl::getOrientation(Eigen::Quaternionf &q) {

  if (!tracker) return;

  auto state = tracker->get();
  if (!state) return;

  auto key = this->key.value_or(gmTrack::StdKey::HEAD_POSE);
  if (!state->contains(key)) return;

  q = state.value()[key].value.orientation;
}

void TrackedViewpoint::setKey(std::string key) {
  _impl->key = key;
}

void TrackedViewpoint::setPoseTracker(std::shared_ptr<gmTrack::PoseTracker> t) {
  _impl->tracker = t;
}

void TrackedViewpoint::traverse(Visitor *visitor) {
  if (_impl->tracker) _impl->tracker->accept(visitor);
}

END_NAMESPACE_GMGRAPHICS;
