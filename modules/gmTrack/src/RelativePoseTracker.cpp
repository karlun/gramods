
#include <gmTrack/RelativePoseTracker.hh>

#include <gmCore/RunOnce.hh>
#include <gmCore/Console.hh>

#include <unordered_set>

BEGIN_NAMESPACE_GMTRACK;

GM_OFI_DEFINE(RelativePoseTracker);
GM_OFI_PARAM2(RelativePoseTracker, originKey, std::string, setOriginKey);
GM_OFI_PARAM2(RelativePoseTracker, relativeKey, std::string, addRelativeKey);
GM_OFI_POINTER2(RelativePoseTracker, originTracker, PoseTracker, addOriginTracker);
GM_OFI_POINTER2(RelativePoseTracker, poseTracker, PoseTracker, addPoseTracker);

struct RelativePoseTracker::Impl {

  std::optional<State> get();

  std::optional<std::string> origin_key;
  std::unordered_set<std::string> relative_keys;
  std::vector<std::shared_ptr<PoseTracker>> origin_tracker;
  std::vector<std::shared_ptr<PoseTracker>> pose_tracker;
};

RelativePoseTracker::RelativePoseTracker() : _impl(std::make_unique<Impl>()) {}
RelativePoseTracker::~RelativePoseTracker() {}

std::optional<PoseTracker::State> RelativePoseTracker::get() {
  return _impl->get();
}

std::optional<PoseTracker::State> RelativePoseTracker::Impl::get() {

  if (pose_tracker.empty()) {
    GM_RUNONCE(GM_WRN("RelativePoseTracker",
                      "Pose requested but no relative tracker available."));
    return std::nullopt;
  }

  if (origin_tracker.empty() && !origin_key) {
    GM_RUNONCE(GM_WRN(
        "RelativePoseTracker",
        "Pose requested but no origin tracker available and no origin key specified to read from relative tracker."));
    return std::nullopt;
  }

  std::optional<PoseTracker::State> relative_state;
  for (auto tracker : pose_tracker)
    PoseTracker::merge(relative_state, tracker->get());

  std::optional<PoseTracker::State> origin_state;
  if (origin_tracker.empty()) {
    origin_state = relative_state;
  } else {
    for (auto tracker : origin_tracker)
      PoseTracker::merge(origin_state, tracker->get());
  }

  if (!relative_state) {
    GM_RUNONCE(GM_WRN("RelativePoseTracker",
                      "Pose requested but missing relative states."));
    return std::nullopt;
  }
  if (!origin_state) {
    GM_RUNONCE(GM_WRN("RelativePoseTracker",
                      "Pose requested but missing origin states."));
    return std::nullopt;
  }

  State state;

  if (origin_key) {
    if (!origin_state->contains(*origin_key)) {
      GM_RUNONCE(GM_WRN("RelativePoseTracker",
                        "Specified origin key ("
                            << *origin_key << ") is not an available pose"));
      return std::nullopt;
    }

    const auto o_sample = origin_state.value()[*origin_key];

    if (relative_keys.empty()) {
      // Transform all keys
      for (const auto as : relative_state.value()) {
        state[as.first] = //
            {.time = std::max(o_sample.time, as.second.time),
             .value = o_sample.value.inverse() * as.second.value};
      }
    } else {
      // Transform only keys listed in relative_keys
      for (const auto as : relative_state.value())
        if (!relative_keys.contains(as.first))
          state[as.first] = as.second;
        else
          state[as.first] = //
              {.time = std::max(o_sample.time, as.second.time),
               .value = o_sample.value.inverse() * as.second.value};
    }
  } else /* (!origin_key) */ {
    for (const auto as : relative_state.value())
      if (origin_state->contains(as.first) &&
          relative_keys.contains(as.first)) {
        const auto o_sample = origin_state.value()[as.first];
        state[as.first] = //
            {.time = std::max(o_sample.time, as.second.time),
             .value = o_sample.value.inverse() * as.second.value};
      } else {
        state[as.first] = as.second;
      }
  }

  return state;
}

void RelativePoseTracker::setOriginKey(std::string key) {
  _impl->origin_key = key;
}

void RelativePoseTracker::addRelativeKey(std::string key) {
  _impl->relative_keys.insert(key);
}

void RelativePoseTracker::addOriginTracker(
    std::shared_ptr<PoseTracker> tracker) {
  _impl->origin_tracker.push_back(tracker);
}

void RelativePoseTracker::addPoseTracker(std::shared_ptr<PoseTracker> tracker) {
  _impl->pose_tracker.push_back(tracker);
}

void RelativePoseTracker::traverse(Visitor *visitor) {
  for (auto tracker : _impl->origin_tracker) tracker->accept(visitor);
  for (auto tracker : _impl->pose_tracker) tracker->accept(visitor);
}

END_NAMESPACE_GMTRACK;

