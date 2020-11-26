
#include <gmTrack/TimeSampleButtonsTracker.hh>

#include <gmCore/Console.hh>
#include <gmCore/RunOnce.hh>

#include <chrono>

BEGIN_NAMESPACE_GMTRACK;

GM_OFI_DEFINE(TimeSampleButtonsTracker);
GM_OFI_PARAM2(TimeSampleButtonsTracker, time, double, addTime);
GM_OFI_PARAM2(TimeSampleButtonsTracker, buttons, size_t, addButtons);


struct TimeSampleButtonsTracker::Impl {

  Impl();

  void addTime(double t);
  void addButtons(size_t b);
  void setState(std::map<size_t, bool>& buttons, size_t state);

  bool getButtons(ButtonsSample &b);

  std::vector<double> time;
  size_t sample_idx = 0;
  std::vector<size_t> button_states;

  std::chrono::steady_clock::time_point start_time;
};

TimeSampleButtonsTracker::TimeSampleButtonsTracker()
  : _impl(new Impl()) {}

TimeSampleButtonsTracker::~TimeSampleButtonsTracker() {}

TimeSampleButtonsTracker::Impl::Impl()
  : start_time(std::chrono::steady_clock::now()) {}


void TimeSampleButtonsTracker::Impl::addTime(double t) { time.push_back(t); }

void TimeSampleButtonsTracker::Impl::addButtons(size_t b) { button_states.push_back(b); }

bool TimeSampleButtonsTracker::Impl::getButtons(ButtonsSample &sample) {

  if (button_states.empty()) {
    GM_RUNONCE(GM_ERR("TimeSampleButtonsTracker", "No button states available."));
    return false;
  }

  if (!time.empty() &&
      button_states.size() >= 2 &&
      time.size() != button_states.size()) {

    GM_RUNONCE(GM_ERR("TimeSampleButtonsTracker", "cannot find button state - sample count mismatch (" << time.size() << " and " << button_states.size() << ")"));

    return false;
  }

  sample.time = ButtonsTracker::clock::now();
  
  if (time.empty()) {
    // If there is no time specified, iterate per frame

    assert(sample_idx < button_states.size());
    setState(sample.buttons, button_states[sample_idx]);

    sample_idx = (sample_idx + 1) % button_states.size();

    return true;
  }

  typedef std::chrono::duration<double, std::ratio<1>> d_seconds;
  double duration = std::chrono::duration_cast<d_seconds>
    (std::chrono::steady_clock::now() - start_time).count();

  if (duration > time.back())
    duration -= int(duration / time.back()) * time.back();

  if (duration <= time.front()) {
    sample.buttons.clear();
    return true;
  }

  size_t last_time = 0;
  for (size_t idx = 1; idx < time.size(); ++idx) {
    if (time[idx] >= duration)
      break;
    last_time = idx;
  }

  setState(sample.buttons, button_states[last_time]);

  return true;
}

void TimeSampleButtonsTracker::Impl::setState(std::map<size_t, bool>& buttons, size_t state) {
  buttons.clear();
  for (size_t idx = 0; state > 0; ++idx) {
    if (state & 1) buttons[idx] = true;
    state >>= 1;
  }
}

void TimeSampleButtonsTracker::addTime(double t) {
  _impl->addTime(t);
}

void TimeSampleButtonsTracker::addButtons(size_t b) {
  _impl->addButtons(b);
}

bool TimeSampleButtonsTracker::getButtons(ButtonsSample &b) {
  return _impl->getButtons(b);
}

END_NAMESPACE_GMTRACK;

