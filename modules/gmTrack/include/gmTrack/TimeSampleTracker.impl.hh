
#include <gmTrack/TimeSampleTracker.hh>

#include <gmCore/Console.hh>
#include <gmCore/io_typeid.hh>
#include <gmCore/PreConditionViolation.hh>
#include <gmCore/RunOnce.hh>
#include <gmCore/Updateable.hh>

#include <optional>
#include <unordered_set>

BEGIN_NAMESPACE_GMTRACK;

template<class TYPE> struct TimeSampleTracker<TYPE>::Impl : gmCore::Updateable {

  typedef typename TrackerBase<TYPE>::State State;

  Impl() : gmCore::Updateable(100) {}

  void update(clock::time_point now, size_t frame) override;

  void processIncoming();
  TYPE interpolate(TYPE a, TYPE b, float r);

  std::vector<std::string> incoming_keys;
  std::vector<TYPE> incoming_values;
  std::vector<double> incoming_times;
  
  size_t timeless_idx = 0;
  std::vector<State> timeless_states;

  struct TimeState {
    double time;
    State state;
  };

  std::optional<std::chrono::steady_clock::time_point> start_time;
  std::vector<TimeState> timed_states;

  std::optional<State> state;
};

template<class TYPE>
TimeSampleTracker<TYPE>::TimeSampleTracker()
  : _impl(std::make_unique<TimeSampleTracker<TYPE>::Impl>()) {}

template<class TYPE> TimeSampleTracker<TYPE>::~TimeSampleTracker() {}

template<class TYPE> void TimeSampleTracker<TYPE>::Impl::processIncoming() {

  if (incoming_keys.size() == 1 && //
      !incoming_values.empty() &&  //
      incoming_times.empty()) {
    // keyA, sample1, sample2, sample3, ...
    GM_DBG2(GM_STR("TimeSampleTracker<" << demangle(typeid(TYPE)) << ">"),
            "processing 1:N:0 where N = " << incoming_values.size());

    if (!timed_states.empty())
      throw gmCore::PreConditionViolation(
          "Cannot add time to timeless states");

    for (const auto v : incoming_values)
      timeless_states.push_back(
          State {{incoming_keys.at(0), {.time = {}, .value = v}}});

  } else if (incoming_keys.size() == incoming_values.size() && //
             incoming_times.empty()) {
    // keyA, sample1, keyB, sample1, keyA, sample2, keyB, sample2, ...
    GM_DBG2(GM_STR("TimeSampleTracker<" << demangle(typeid(TYPE)) << ">"),
            "processing N:N:0 where N = " << incoming_keys.size());

    if (!timed_states.empty())
      throw gmCore::PreConditionViolation(
          "Cannot add time to timeless states");

    for (size_t idx = 0; idx < incoming_keys.size(); ++idx)
      timeless_states.push_back(State {{incoming_keys.at(idx), //
                                        {.value = incoming_values.at(idx)}}});

  } else if (incoming_keys.size() == 1 && //
             incoming_values.size() == incoming_times.size()) {
    // keyA, sample1, time1, sample2, time2, ...
    GM_DBG2(GM_STR("TimeSampleTracker<" << demangle(typeid(TYPE)) << ">"),
            "processing 1:N:N where N = " << incoming_values.size());

    if (!timeless_states.empty())
      throw gmCore::PreConditionViolation(
          "Cannot add time to timeless states");

    for (size_t idx = 0; idx < incoming_keys.size(); ++idx) {
      if (!timed_states.empty() &&
          incoming_times.at(idx) <= timed_states.back().time)
        throw gmCore::PreConditionViolation(GM_STR(
            "Sample time must be increasing; "
            << incoming_times.at(idx) << " <= " << timed_states.back().time));
      timed_states.push_back(
          {.time = incoming_times.at(idx),
           .state = State {{incoming_keys.at(0), //
                            {.value = incoming_values.at(idx)}}}});
    }

  } else if (incoming_times.size() > 1 &&
             (incoming_keys.size() / incoming_times.size()) ==
                 (incoming_values.size() / incoming_times.size()) &&
             (incoming_keys.size() % incoming_times.size()) == 0 &&
             (incoming_values.size() % incoming_times.size()) == 0) {
    // keyA, sample1, keyB, sample1, time1, keyA, sample2, keyB, sample2, time2, ...
    const auto N = incoming_times.size();
    const auto M = (incoming_keys.size() / incoming_times.size());
    GM_DBG2(GM_STR("TimeSampleTracker<" << demangle(typeid(TYPE)) << ">"),
            "processing NM:NM:N where N = " << incoming_times.size()
                                            << " and M = " << M);

    if (!timeless_states.empty())
      throw gmCore::PreConditionViolation(
          "Cannot add time to timeless states");

    for (size_t idx_N = 0; idx_N < N; ++idx_N) {
      State state;
      for (size_t idx_M = 0; idx_M < M; ++idx_M) {
        const auto idx = idx_M + M * idx_N;
        if (state.contains(incoming_keys.at(idx)))
          throw gmCore::RuntimeException(
              GM_STR("Adding key twice (" << demangle(typeid(TYPE))
                                          << "): " << incoming_keys.at(idx)));
        state.insert(
            {incoming_keys.at(idx), {.value = incoming_values.at(idx)}});
      }
      if (!timed_states.empty() &&
          incoming_times.at(idx_N) <= timed_states.back().time)
        throw gmCore::PreConditionViolation(
            GM_STR("Sample time must be increasing; "
                   << incoming_times.at(idx_N)
                   << " <= " << timed_states.back().time));
      timed_states.push_back(
          {.time = incoming_times.at(idx_N), .state = state});
    }

  } else {
    throw gmCore::RuntimeException(
        GM_STR("Invalid time sample data ("
               << demangle(typeid(TYPE))
               << "): key, state, time count = " << incoming_keys.size() << ", "
               << incoming_values.size() << ", " << incoming_times.size()));
  }

  incoming_keys.clear();
  incoming_values.clear();
  incoming_times.clear();
}

template<class TYPE>
void TimeSampleTracker<TYPE>::Impl::update(clock::time_point now,
                                           size_t frame) {
  if (!incoming_keys.empty() ||   //
      !incoming_values.empty() || //
      !incoming_times.empty()) [[unlikely]]
    processIncoming();

  if (timeless_states.empty() && timed_states.empty()) {
    GM_RUNONCE(
        GM_ERR(GM_STR("TimeStateTracker<" << demangle(typeid(TYPE)) << ">"),
               "No states available."));
    return;
  }

  if (!timeless_states.empty()) {
    state = timeless_states[timeless_idx];
    for (auto &s : state.value()) s.second.time = now;
    timeless_idx = (timeless_idx + 1) % timeless_states.size();
    return;
  }

  if (!start_time) start_time = now;

  typedef std::chrono::duration<double, std::ratio<1>> d_seconds;
  double duration = std::chrono::duration_cast<d_seconds>(
                        std::chrono::steady_clock::now() - *start_time)
                        .count();

  if (duration > timed_states.back().time) {
    auto back_time = timed_states.back().time;
    duration -= int(duration / back_time) * back_time;
  }

  if (duration < timed_states.front().time) return;

  size_t last_idx = 0;
  for (size_t idx = 0; idx < timed_states.size(); ++idx) {
    if (timed_states[idx].time > duration) break;
    last_idx = idx;
  }

  float r =
      float((duration - timed_states[last_idx].time) /
            (timed_states[last_idx + 1].time - timed_states[last_idx].time));

  state = timed_states[last_idx].state;
  for (auto &s : state.value()) {
    s.second.time = now;
    if (timed_states[last_idx + 1].state.contains(s.first)) {
      s.second.value =
          interpolate(s.second.value,
                      timed_states[last_idx + 1].state.at(s.first).value,
                      r);
    }
  }
}

template<class TYPE>
void TimeSampleTracker<TYPE>::addKey(std::string key) {
  _impl->incoming_keys.push_back(key);
}

template<class TYPE> void TimeSampleTracker<TYPE>::addValue(TYPE v) {
  _impl->incoming_values.push_back(v);
}

template<class TYPE> void TimeSampleTracker<TYPE>::addTime(double seconds) {
  _impl->incoming_times.push_back(seconds);
}

template<class TYPE> std::optional<typename TrackerBase<TYPE>::State> TimeSampleTracker<TYPE>::get() {
  return _impl->state;
}

END_NAMESPACE_GMTRACK;
