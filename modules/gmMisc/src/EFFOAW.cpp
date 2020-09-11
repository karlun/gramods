
#include <gmMisc/EFFOAW.hh>

#ifdef gramods_ENABLE_Eigen3

#include <limits>

BEGIN_NAMESPACE_GMMISC;

struct EFFOAW::Impl {

  Impl();

  double getLastSampleTime(size_t id, size_t N);

  void addSample(size_t id, Eigen::Vector3d position, double time);

  void removeLastSample(size_t id);

  Eigen::Vector3d estimateVelocity(size_t id, double error_threshold, size_t *samples) const;

  Eigen::Vector3d estimatePosition(size_t id, double error_threshold, double time, size_t *ret_samples) const;

  void cleanup(double time);

  /** The number of samples that are saved. */
  size_t history_length;

  /** How old samples should be saved. */
  double history_duration;

  /** Type of list containing time values. */
  typedef std::deque<double> time_list_t;

  /** Type of list containing position values. */
  typedef std::deque<Eigen::Vector3d> position_list_t;

  /** Type combining list of time values with list of position values. */
  typedef std::pair<time_list_t, position_list_t> time_position_t;

  /** History of points, per id. */
  std::map<size_t, time_position_t> history;
};


EFFOAW::EFFOAW()
  : _impl(std::make_unique<Impl>()) {}

EFFOAW::~EFFOAW() {}

EFFOAW::Impl::Impl()
  : history_length(10),
    history_duration(1.f) {}

void EFFOAW::setHistoryLength(size_t N){
  _impl->history_length = N; }

size_t EFFOAW::getHistoryLength() const {
  return _impl->history_length; }

void EFFOAW::setHistoryDuration(double t){
  _impl->history_duration = t; }

double EFFOAW::getHistoryDuration() const {
  return _impl->history_duration; }

double EFFOAW::getLastSampleTime(size_t id, size_t N) {
  return _impl->getLastSampleTime(id, N);
}

double EFFOAW::Impl::getLastSampleTime(size_t id, size_t N) {
  if (history.find(id) == history.end() ||
      history[id].first.size() < 1 + N)
    return -1;
  return history[id].first[N];
}

void EFFOAW::addSample(size_t id, Eigen::Vector3d position, double time){
  _impl->addSample(id, position, time);
}

void EFFOAW::Impl::addSample(size_t id, Eigen::Vector3d position, double time){

  time_list_t &time_list = history[id].first;
  position_list_t &position_list = history[id].second;

  // Remove old samples that have the same time (override old sample)
  while (! time_list.empty() && fabs(time - time_list.front()) < std::numeric_limits<double>::epsilon()) {
    position_list.pop_front();
    time_list.pop_front();
  }

  position_list.push_front(position);
  time_list.push_front(time);
}

void EFFOAW::removeLastSample(size_t id) {
  _impl->removeLastSample(id);
}

void EFFOAW::Impl::removeLastSample(size_t id) {
  history[id].first.pop_front();
  history[id].second.pop_front();
}

Eigen::Vector3d EFFOAW::estimateVelocity
(size_t id, double error_threshold, size_t *samples) const {
  return _impl->estimateVelocity(id, error_threshold, samples);
}

Eigen::Vector3d EFFOAW::Impl::estimateVelocity
(size_t id, double error_threshold, size_t *samples) const {

  typename std::map< size_t, time_position_t >::const_iterator hist = history.find(id);
  if (hist == history.end()) {
    if (samples != nullptr)
      *samples = 0;
    return Eigen::Vector3d::Zero();
  }

  const time_list_t & time_list = hist->second.first;
  const position_list_t & position_list = hist->second.second;

  if (position_list.size() < 2) {
    if (samples != nullptr)
      *samples = 0;
    return Eigen::Vector3d::Zero();
  }

  size_t best_valid_n = 1;
  for (size_t n = 2 ; n < position_list.size() ; ++n) {

    Eigen::Vector3d diff = (position_list[0] - position_list[n]) / (time_list[0] - time_list[n]);

    bool is_valid = true;
    for (size_t i = 1 ; i <= n ; ++i) {

      Eigen::Vector3d offset = (time_list[0] - time_list[i]) * diff;
      Eigen::Vector3d pos = position_list[0] - position_list[i] - offset;
      double err2 = pos.squaredNorm();
      if (err2 > error_threshold * error_threshold) {
        is_valid = false;
        break;
      }
    }

    if (is_valid) {
      best_valid_n = n; }
    else {
      break; }
  }

  if (samples != nullptr)
    *samples = best_valid_n + 1;

  // (pos[0] - pos[best_n])/(t[0] - t[best_n])
  return (position_list[0] - position_list[best_valid_n])
    / (time_list[0] - time_list[best_valid_n]);
}

Eigen::Vector3d EFFOAW::estimatePosition(size_t id, double error_threshold, double time, size_t *ret_samples) const {
  return _impl->estimatePosition(id, error_threshold, time, ret_samples);
}

Eigen::Vector3d EFFOAW::Impl::estimatePosition(size_t id, double error_threshold, double time, size_t *ret_samples) const {

  typename std::map< size_t, time_position_t >::const_iterator hist = history.find(id);
  if (hist == history.end()) {
    if (ret_samples != nullptr)
      *ret_samples = 0;
    return Eigen::Vector3d::Zero();
  }

  const time_list_t & time_list = hist->second.first;
  const position_list_t & position_list = hist->second.second;

  if (position_list.size() == 0) {
    if (ret_samples != nullptr)
      *ret_samples = 0;
    return Eigen::Vector3d::Zero();
  }

  size_t samples;
  Eigen::Vector3d velocity = estimateVelocity(id, error_threshold, &samples);

  // Two samples should give position identical to p0, minus floating
  // point errors
  if (samples <= 2) {
    if (ret_samples != nullptr)
      *ret_samples = 1;
    return position_list[0];
  }

  if (ret_samples != nullptr)
    *ret_samples = samples;

  double sum_time = 0.0;
  Eigen::Vector3d sum_position = Eigen::Vector3d::Zero();
  for (size_t idx = 0; idx < samples; ++idx) {
    sum_time += time_list[idx];
    sum_position += position_list[idx];
  }

  double avg_time = sum_time / samples;
  Eigen::Vector3d avg_position = sum_position * (1.0 / samples);

  if (ret_samples != nullptr)
    *ret_samples = samples;

  return avg_position + (time - avg_time) * velocity;
}

void EFFOAW::cleanup(double time){
  _impl->cleanup(time);
}

void EFFOAW::Impl::cleanup(double time) {

  if (time < 0)
    for (auto it : history)
      if (it.second.first.size() > 0 && it.second.first[0] > time)
        time = it.second.first[0];

  for (typename std::map< size_t, time_position_t >::iterator it = history.begin() ;
       it != history.end() ;) {

    time_list_t &time_list = it->second.first;
    position_list_t &position_list = it->second.second;

    while (time_list.size() > history_length) {
      position_list.pop_back();
      time_list.pop_back();
    }

    while (!time_list.empty() && (time - time_list.back()) > history_duration) {
      position_list.pop_back();
      time_list.pop_back();
    }

    typename std::map< size_t, time_position_t >::iterator current_it = it;
    it++;

    if (time_list.empty()) {
      history.erase(current_it);
    }
  }
}

END_NAMESPACE_GMMISC;

#endif
