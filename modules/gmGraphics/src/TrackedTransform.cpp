
#include <gmGraphics/TrackedTransform.hh>

#include <gmCore/Updateable.hh>
#include <gmCore/TimeTools.hh>

#include <unordered_set>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE_SUB(TrackedTransform, Group);
GM_OFI_PARAM2(TrackedTransform, hysteresis, float, setHysteresis);
GM_OFI_PARAM2(TrackedTransform, key, std::string, addKey);
GM_OFI_POINTER2(TrackedTransform, poseTracker, gmTrack::PoseTracker, addPoseTracker);

struct TrackedTransform::Impl : gmCore::Updateable {

  void update(clock::time_point t, size_t frame) override;
  void accept(Node *node, Visitor *visitor);

  clock::time_point now_time;
  clock::duration hysteresis =
      gmCore::TimeTools::secondsToDuration(double(0.5));

  std::vector<std::shared_ptr<gmTrack::PoseTracker>> pose_trackers;
  std::unordered_set<std::string> keys;
};

TrackedTransform::TrackedTransform() : _impl(std::make_unique<Impl>()) {}

TrackedTransform::~TrackedTransform() {}

void TrackedTransform::Impl::update(clock::time_point t, size_t frame) {
  now_time = t;
}

void TrackedTransform::setHysteresis(float t) {
  _impl->hysteresis = gmCore::TimeTools::secondsToDuration(double(t));
}

void TrackedTransform::addKey(std::string key) {
  _impl->keys.insert(key);
}

void TrackedTransform::addPoseTracker(
    std::shared_ptr<gmTrack::PoseTracker> t) {
  _impl->pose_trackers.push_back(t);
}

void TrackedTransform::accept(Visitor *visitor) {
  _impl->accept(this, visitor);
}

void TrackedTransform::Impl::accept(Node *node, Visitor *visitor) {
  auto *ts_visitor = dynamic_cast<Node::TransformStackVisitor *>(visitor);
  if (!ts_visitor) {
    visitor->apply(node);
    return;
  }

  for (auto &tracker : pose_trackers) {
    auto state = tracker->get();
    if (!state) continue;

    if (keys.empty()) {
      for (const auto as : state.value()) {
        if (now_time - as.second.time > hysteresis) continue;
        ts_visitor->apply(node, as.second.value.asMatrix());
      }
    } else {
      for (const auto key : keys)
        if (state->contains(key) &&
            now_time - state->at(key).time <= hysteresis)
          ts_visitor->apply(node, state->at(key).value.asMatrix());
    }
  }
}

END_NAMESPACE_GMGRAPHICS;
