
#include <gmGraphics/TrackedTransform.hh>

#include <gmCore/Updateable.hh>
#include <gmCore/TimeTools.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE_SUB(TrackedTransform, Group);
GM_OFI_PARAM2(TrackedTransform, hysteresis, float, setHysteresis);
GM_OFI_POINTER2(TrackedTransform, singlePoseTracker, gmTrack::SinglePoseTracker, addSinglePoseTracker);
GM_OFI_POINTER2(TrackedTransform, multiPoseTracker, gmTrack::MultiPoseTracker, addMultiPoseTracker);

struct TrackedTransform::Impl : gmCore::Updateable {

  void update(clock::time_point t, size_t frame) override;
  void accept(Node *node, Visitor *visitor);

  clock::time_point now_time;
  clock::duration hysteresis =
      gmCore::TimeTools::secondsToDuration(double(0.5));

  std::vector<std::shared_ptr<gmTrack::SinglePoseTracker>> single_pose_trackers;
  std::vector<std::shared_ptr<gmTrack::MultiPoseTracker>> multi_pose_trackers;
};

TrackedTransform::TrackedTransform() : _impl(std::make_unique<Impl>()) {}

TrackedTransform::~TrackedTransform() {}

void TrackedTransform::Impl::update(clock::time_point t, size_t frame) {
  now_time = t;
}

void TrackedTransform::setHysteresis(float t) {
  _impl->hysteresis = gmCore::TimeTools::secondsToDuration(double(t));
}

void TrackedTransform::addSinglePoseTracker(
    std::shared_ptr<gmTrack::SinglePoseTracker> t) {
  _impl->single_pose_trackers.push_back(t);
}

void TrackedTransform::addMultiPoseTracker(
    std::shared_ptr<gmTrack::MultiPoseTracker> t) {
  _impl->multi_pose_trackers.push_back(t);
}

void TrackedTransform::accept(Visitor *visitor) {
  _impl->accept(this, visitor);
}

void TrackedTransform::Impl::accept(Node *node, Visitor *visitor) {
  if (auto *ts_visitor =
          dynamic_cast<Node::TransformStackVisitor *>(visitor)) {
    for (auto &tracker : single_pose_trackers) {
      gmTrack::PoseTracker::PoseSample pose;
      if (!tracker->getPose(pose)) continue;
      if (now_time - pose.time > hysteresis) continue;

      ts_visitor->apply(node, pose.asMatrix());
    }

    for (auto &tracker : multi_pose_trackers) {
      std::map<int, gmTrack::PoseTracker::PoseSample> poses;
      if (!tracker->getPose(poses)) continue;

      for (const auto &pose : poses) {
        if (now_time - pose.second.time > hysteresis) continue;

        ts_visitor->apply(node, pose.second.asMatrix());
      }
    }
  } else {
    visitor->apply(node);
  }
}

END_NAMESPACE_GMGRAPHICS;
