
#include <gmTrack/RelativePoseTracker.hh>

#include <gmCore/RunOnce.hh>
#include <gmCore/Console.hh>

#include <unordered_set>

BEGIN_NAMESPACE_GMTRACK;

GM_OFI_DEFINE(RelativePoseTracker);
GM_OFI_PARAM2(RelativePoseTracker, originKey, std::string, setOriginKey);
GM_OFI_PARAM2(RelativePoseTracker, relativeKey, std::string, addRelativeKey);
GM_OFI_POINTER2(RelativePoseTracker, originTracker, PoseTracker, setOriginTracker);
GM_OFI_POINTER2(RelativePoseTracker, poseTracker, PoseTracker, setPoseTracker);

struct RelativePoseTracker::Impl {

  std::optional<State> get();

  std::optional<std::string> origin_key;
  std::unordered_set<std::string> relative_keys;
  std::shared_ptr<PoseTracker> origin_tracker;
  std::shared_ptr<PoseTracker> pose_tracker;
};

RelativePoseTracker::RelativePoseTracker() : _impl(std::make_unique<Impl>()) {}
RelativePoseTracker::~RelativePoseTracker() {}

std::optional<PoseTracker::State> RelativePoseTracker::get() {
  return _impl->get();
}

std::optional<PoseTracker::State> RelativePoseTracker::Impl::get() {

  if (!pose_tracker) {
    GM_RUNONCE(GM_WRN("RelativePoseTracker",
                      "Pose requested but no relative tracker available."));
    return std::nullopt;
  }

  if (!origin_tracker && !origin_key) {
    GM_RUNONCE(GM_WRN(
        "RelativePoseTracker",
        "Pose requested but no origin tracker available and no origin key specified to read from relative tracker."));
    return std::nullopt;
  }

  auto relative_state = pose_tracker->get();
  auto origin_state = origin_tracker ? origin_tracker->get() : relative_state;

  if (!relative_state || !origin_state) {
    GM_RUNONCE(
        GM_WRN("RelativePoseTracker", "Pose requested but missing states."));
    return std::nullopt;
  }

  State state;

  if (origin_key) {
    if (!origin_state->contains(*origin_key))
      return std::nullopt;
    const auto o_sample = origin_state.value()[*origin_key];

    if (relative_keys.empty()) {
      // Transform all keys
      for (const auto as : relative_state.value())
        state[as.first] = {
            .time = std::max(o_sample.time, as.second.time),
            .value = {.position =
                          o_sample.value.orientation.conjugate() *
                          (as.second.value.position - o_sample.value.position),
                      .orientation = o_sample.value.orientation.conjugate() *
                                     as.second.value.orientation}};
    } else {
      // Transform only keys listed in relative_keys
      for (const auto as : relative_state.value())
        if (!relative_keys.contains(as.first))
          state[as.first] = as.second;
        else
          state[as.first] = {
              .time = std::max(o_sample.time, as.second.time),
              .value = {.position = o_sample.value.orientation.conjugate() *
                                    (as.second.value.position -
                                     o_sample.value.position),
                        .orientation = o_sample.value.orientation.conjugate() *
                                       as.second.value.orientation}};
    }
  } else {
    for (const auto as : relative_state.value())
      if (origin_state->contains(as.first) &&
          relative_keys.contains(as.first)) {
        const auto o_sample = origin_state.value()[as.first];
        state[as.first] = {
            .time = std::max(o_sample.time, as.second.time),
            .value = {.position =
                          o_sample.value.orientation.conjugate() *
                          (as.second.value.position - o_sample.value.position),
                      .orientation = o_sample.value.orientation.conjugate() *
                                     as.second.value.orientation}};
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

void RelativePoseTracker::setOriginTracker(std::shared_ptr<PoseTracker> tracker) {
  _impl->origin_tracker = tracker;
}

void RelativePoseTracker::setPoseTracker(std::shared_ptr<PoseTracker> tracker) {
  _impl->pose_tracker = tracker;
}

void RelativePoseTracker::traverse(Visitor *visitor) {
  if (_impl->origin_tracker) _impl->origin_tracker->accept(visitor);
  if (_impl->pose_tracker) _impl->pose_tracker->accept(visitor);
}

END_NAMESPACE_GMTRACK;

