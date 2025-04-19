
#include <gmTrack/TimeSampleAnalogsTracker.hh>

#include <gmCore/Console.hh>
#include <gmCore/Updateable.hh>
#include <gmCore/RunOnce.hh>

#include <chrono>

BEGIN_NAMESPACE_GMTRACK;

GM_OFI_DEFINE(TimeSampleAnalogsTracker);
GM_OFI_PARAM2(TimeSampleAnalogsTracker, time, double, addTime);
GM_OFI_PARAM2(TimeSampleAnalogsTracker, analogs, gmCore::float3, addAnalogs);


struct TimeSampleAnalogsTracker::Impl : gmCore::Updateable {

  Impl() : gmCore::Updateable(100) {}

  void update(clock::time_point now, size_t frame) override;

  void addTime(double t);
  void addAnalogs(gmCore::float3 a);

  bool getAnalogs(AnalogsSample &a);

  std::vector<double> time;
  size_t sample_idx = 0;
  std::vector<gmCore::float3> states;

  std::optional<AnalogsSample> analogs_sample;

  std::optional<std::chrono::steady_clock::time_point> start_time;
};

TimeSampleAnalogsTracker::TimeSampleAnalogsTracker()
  : _impl(new Impl()) {}

TimeSampleAnalogsTracker::~TimeSampleAnalogsTracker() {}

void TimeSampleAnalogsTracker::Impl::addTime(double t) { time.push_back(t); }

void TimeSampleAnalogsTracker::Impl::addAnalogs(gmCore::float3 a) { states.push_back(a); }

bool TimeSampleAnalogsTracker::Impl::getAnalogs(AnalogsSample &sample) {
  if (!analogs_sample) return false;
  sample = *analogs_sample;
  return true;
}

void TimeSampleAnalogsTracker::Impl::update(clock::time_point now,
                                            size_t frame) {
  if (states.empty()) {
    GM_RUNONCE(GM_ERR("TimeSampleAnalogsTracker", "No analog samples available."));
    return;
  }

  if (!time.empty() && states.size() >= 2 && time.size() != states.size()) {
    GM_RUNONCE(GM_ERR("TimeSampleAnalogsTracker", "cannot find state - sample count mismatch (" << time.size() << " and " << states.size() << ")"));

    return;
  }

  if (!start_time) start_time = now;
  analogs_sample = AnalogsSample {};
  analogs_sample->time = now;

  if (time.empty()) {
    // If there is no time specified, iterate per frame

    for (auto s : states[sample_idx])
      analogs_sample->analogs.push_back(s);

    sample_idx = (sample_idx + 1) % states.size();

    return;
  }

  typedef std::chrono::duration<double, std::ratio<1>> d_seconds;
  double duration = std::chrono::duration_cast<d_seconds>(
                        std::chrono::steady_clock::now() - *start_time)
                        .count();

  if (duration > time.back()) {
    duration -= int(duration/time.back()) * time.back();
  }

  size_t to_time = 0;
  for (size_t idx = 0; idx < time.size(); ++idx) {
    if (time[idx] > duration) {
      to_time = idx;
      break;
    }
  }

  size_t from_time = to_time - 1;
  float a =
      float((duration - time[from_time]) / (time[to_time] - time[from_time]));

  for (size_t idx = 0; idx < 3; ++idx)
    analogs_sample->analogs.push_back(a * states[to_time][idx] +
                                      (1 - a) * states[from_time][idx]);
}

void TimeSampleAnalogsTracker::addTime(double t) {
  _impl->addTime(t);
}

void TimeSampleAnalogsTracker::addAnalogs(gmCore::float3 a) {
  _impl->addAnalogs(a);
}

bool TimeSampleAnalogsTracker::getAnalogs(AnalogsSample &a) {
  return _impl->getAnalogs(a);
}

END_NAMESPACE_GMTRACK;

