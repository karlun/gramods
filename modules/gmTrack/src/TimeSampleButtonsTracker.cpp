
#include <gmTrack/TimeSampleButtonsTracker.hh>

#include <gmCore/Console.hh>
#include <gmCore/RunOnce.hh>

#include <chrono>

BEGIN_NAMESPACE_GMTRACK;

GM_OFI_DEFINE(TimeSampleButtonsTracker);
GM_OFI_PARAM(TimeSampleButtonsTracker, time, double, TimeSampleButtonsTracker::addTime);
GM_OFI_PARAM(TimeSampleButtonsTracker, buttons, int, TimeSampleButtonsTracker::addButtons);


struct TimeSampleButtonsTracker::Impl {

  Impl();

  void addTime(double t);
  void addButtons(int b);

  bool getButtons(ButtonsSample &b);

  std::vector<double> time;
  size_t sample_idx;
  std::vector<int> button_states;

  std::chrono::steady_clock::time_point start_time;
};

TimeSampleButtonsTracker::TimeSampleButtonsTracker()
  : _impl(new Impl()) {}

TimeSampleButtonsTracker::~TimeSampleButtonsTracker() {}

TimeSampleButtonsTracker::Impl::Impl()
  : start_time(std::chrono::steady_clock::now()),
    sample_idx(-1) {}


void TimeSampleButtonsTracker::Impl::addTime(double t) { time.push_back(t); }

void TimeSampleButtonsTracker::Impl::addButtons(int b) { button_states.push_back(b); }

bool TimeSampleButtonsTracker::Impl::getButtons(ButtonsSample &b) {

  if (!time.empty() && button_states.size() >= 2 && time.size() != button_states.size()) {

    GM_RUNONCE(GM_ERR("TimeSampleButtonsTracker", "cannot find button state - sample count mismatch (" << time.size() << " and " << button_states.size() << ")"));

    return false;
  }

  b.time = ButtonsTracker::clock::now();
  
  if (time.empty()) {
    // If there is not time specified, iterate per frame

    sample_idx = (sample_idx + 1) % button_states.size();

    if (button_states.size() == 0) {
      b.buttons = 0;
    } else {
      assert(sample_idx < button_states.size());
      b.buttons = button_states[sample_idx];
    }

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

  if (button_states.size() == 0)
    b.buttons = 0;
  else
    b.buttons = button_states[last_time];

  return true;
}


void TimeSampleButtonsTracker::addTime(double t) {
  _impl->addTime(t);
}

void TimeSampleButtonsTracker::addButtons(int b) {
  _impl->addButtons(b);
}

bool TimeSampleButtonsTracker::getButtons(ButtonsSample &b) {
  return _impl->getButtons(b);
}

END_NAMESPACE_GMTRACK;

