
#include <gmTrack/TimeSampleAnalogsTracker.hh>

#include <gmCore/Console.hh>

#include <chrono>

BEGIN_NAMESPACE_GMTRACK;

GM_OFI_DEFINE(TimeSampleAnalogsTracker);
GM_OFI_PARAM(TimeSampleAnalogsTracker, time, double, TimeSampleAnalogsTracker::addTime);
GM_OFI_PARAM(TimeSampleAnalogsTracker, analogs, gmTypes::float3, TimeSampleAnalogsTracker::addAnalogs);


struct TimeSampleAnalogsTracker::Impl {

  Impl();

  void addTime(double t);
  void addAnalogs(gmTypes::float3 a);

  bool getAnalogs(AnalogsSample &a);

  std::vector<double> time;
  size_t sample_idx;
  std::vector<gmTypes::float3> states;

  std::chrono::steady_clock::time_point start_time;
};

TimeSampleAnalogsTracker::TimeSampleAnalogsTracker()
  : _impl(new Impl()) {}

TimeSampleAnalogsTracker::Impl::Impl()
  : start_time(std::chrono::steady_clock::now()),
    sample_idx(-1) {}


void TimeSampleAnalogsTracker::Impl::addTime(double t) { time.push_back(t); }

void TimeSampleAnalogsTracker::Impl::addAnalogs(gmTypes::float3 a) { states.push_back(a); }

bool TimeSampleAnalogsTracker::Impl::getAnalogs(AnalogsSample &b) {

  if (!time.empty() && states.size() >= 2 && time.size() != states.size()) {

    static bool message_shown = false;
    if (!message_shown) {
      GM_ERR("TimeSampleAnalogsTracker", "cannot find state - sample count mismatch (" << time.size() << " and " << states.size() << ")");
      message_shown = true;
    }

    return false;
  }

  if (states.empty())
      return false;

  b.time = AnalogsTracker::clock::now();
  b.analogs.clear();

  if (time.empty()) {
    // If there is not time specified, iterate per frame

    sample_idx = (sample_idx + 1) % states.size();

    assert(sample_idx < states.size());
    for (auto s : states[sample_idx])
      b.analogs.push_back(s);

    return true;
  }

  typedef std::chrono::duration<double, std::ratio<1>> d_seconds;
  auto duration = std::chrono::duration_cast<d_seconds>
    (std::chrono::steady_clock::now() - start_time).count();

  if (duration > time.back()) {
    duration -= int(duration/time.back()) * time.back();
  }

  size_t last_time = 0;
  for (int idx = 0; idx < time.size(); ++idx) {
    if (time[idx] > duration) {
      last_time = idx;
      break;
    }
  }

  assert(last_time >= 0);
  assert(last_time < states.size());

  for (auto s : states[sample_idx])
    b.analogs.push_back(s);
}


void TimeSampleAnalogsTracker::addTime(double t) {
  _impl->addTime(t);
}

void TimeSampleAnalogsTracker::addAnalogs(gmTypes::float3 a) {
  _impl->addAnalogs(a);
}

bool TimeSampleAnalogsTracker::getAnalogs(AnalogsSample &a) {
  return _impl->getAnalogs(a);
}

END_NAMESPACE_GMTRACK;

