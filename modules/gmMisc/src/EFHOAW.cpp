
#include <gmMisc/EFHOAW.hh>

#ifdef gramods_ENABLE_Eigen3

#include <gmCore/InvalidArgument.hh>

#include <deque>
#include <map>
#include <iostream>
#include <limits>

BEGIN_NAMESPACE_GMMISC;

struct EFHOAW::Impl {

  typedef Eigen::Matrix<double, 3, Eigen::Dynamic> polco;

  Impl();

  double getLastSampleTime(size_t id);

  void addSample(size_t id, Eigen::Vector3d position, double time, bool replace);

  polco estimateCoefficients
  (size_t id, double error, size_t order = 2, size_t *samples = nullptr);

  polco findBestFit(size_t id, size_t sample_count, size_t order) const;

  Eigen::MatrixXd getPosVector(size_t id, size_t sample_count, size_t dim) const;

  Eigen::Vector3d getPolynomialPosition(int id, double t) const;
  Eigen::Vector3d getPolynomialVelocity(int id, double t) const;

  static Eigen::Vector3d getPolynomialPosition(polco coefficients, double t);
  static Eigen::Vector3d getPolynomialVelocity(polco coefficients, double t);

  void cleanup(double time);

  /** The number of samples that are saved. */
  size_t history_length = std::numeric_limits<size_t>::max();

  /** How old samples should be saved. */
  double history_duration = std::numeric_limits<double>::max();

  /** Type of list containing time values. */
  typedef std::deque<double> time_list_t;

  /** Type of list containing position values. */
  typedef std::deque<Eigen::Vector3d> position_list_t;

  /** Type combining list of time values with list of position values. */
  typedef std::pair<time_list_t, position_list_t> time_position_t;

  /** History of points, per id. */
  std::map<size_t, time_position_t> history;

  /** Cache of polynomial coefficients. */
  std::map<size_t, polco> coefficients;
};


EFHOAW::EFHOAW()
  : _impl(std::make_unique<Impl>()) {}

EFHOAW::~EFHOAW() {}

EFHOAW::Impl::Impl() {}

void EFHOAW::setHistoryLength(size_t N){
  _impl->history_length = N; }

size_t EFHOAW::getHistoryLength() const {
  return _impl->history_length; }

void EFHOAW::setHistoryDuration(double t){
  _impl->history_duration = t; }

double EFHOAW::getHistoryDuration() const {
  return _impl->history_duration; }

double EFHOAW::getLastSampleTime(size_t id) {
  return _impl->getLastSampleTime(id);
}

double EFHOAW::Impl::getLastSampleTime(size_t id) {
  if (history.find(id) == history.end() ||
      history.at(id).first.size() < 1)
    return -1;
  return history.at(id).first[0];
}

void EFHOAW::addSample(size_t id, Eigen::Vector3d position, double time, bool replace){
  _impl->addSample(id, position, time, replace);
}

void EFHOAW::Impl::addSample
(size_t id, Eigen::Vector3d position, double time, bool replace) {

  time_list_t &time_list = history[id].first;
  position_list_t &position_list = history[id].second;

  if (replace) {
    // Remove old samples that have the same time (override old sample)
    while (! time_list.empty() && fabs(time - time_list.front()) <
           std::numeric_limits<double>::epsilon()) {
      position_list.pop_front();
      time_list.pop_front();
    }
  }

  position_list.push_front(position);
  time_list.push_front(time);

  while (position_list.size() > history_length) {
    position_list.pop_back();
    time_list.pop_back();
  }

  while (!time_list.empty() && time - time_list.back() > history_duration) {
    position_list.pop_back();
    time_list.pop_back();
  }

  coefficients.erase(id);
}

EFHOAW::polco EFHOAW::estimateCoefficients
(size_t id, double error, size_t order, size_t *samples) {
  return _impl->estimateCoefficients(id, error, order, samples);
}

EFHOAW::polco EFHOAW::Impl::estimateCoefficients
(size_t id, double error, size_t order, size_t *samples) {

  typename std::map< size_t, time_position_t >::const_iterator hist = history.find(id);
  if (hist == history.end()) {
    if (samples != nullptr)
      *samples = 0;
    return polco(3, 0);
  }

  const time_list_t & time_list = hist->second.first;
  const position_list_t & position_list = hist->second.second;

  if (position_list.size() < order + 1) {
    if (samples != nullptr)
      *samples = 0;
    return polco(3, 0);
  }

  size_t best_count = 0;
  polco best_coefficients;

  for (size_t candidate_count = order + 1; candidate_count <= position_list.size(); ++candidate_count) {

    polco coefficients = findBestFit(id, candidate_count, order);

    bool is_valid = true;
    for (size_t idx = 0; idx < candidate_count; ++idx) {

      auto pos = getPolynomialPosition(coefficients, time_list.at(idx) - time_list.at(0));
      auto off = pos - position_list.at(idx);

      if (off.norm() > error) {
        is_valid = false;
        break;
      }
    }

    if (is_valid) {
      best_count = candidate_count;
      best_coefficients = coefficients;
    } else {
      break;
    }
  }

  if (samples != nullptr)
    *samples = best_count;

  if (best_count == 0)
    return polco(3, 0);

  return coefficients[id] = best_coefficients;
}

void EFHOAW::cleanup(double time){
  _impl->cleanup(time);
}

void EFHOAW::Impl::cleanup(double time) {

  if (time < 0)
    for (auto it : history)
      if (it.second.first.size() > 0 && it.second.first[0] > time)
        time = it.second.first[0];

  for (typename std::map< size_t, time_position_t >::iterator it = history.begin() ;
       it != history.end() ;) {

    time_list_t &time_list = it->second.first;
    position_list_t &position_list = it->second.second;
    coefficients.erase(it->first);

    while (!time_list.empty() && time_list.size() > history_length) {
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

EFHOAW::polco EFHOAW::Impl::findBestFit(size_t id, size_t sample_count, size_t order) const {

  const time_list_t &time_list = history.at(id).first;
  Eigen::MatrixXd poly(sample_count, order + 1);

  for (size_t sample_idx = 0; sample_idx < sample_count; ++sample_idx) {
    double t = time_list[sample_idx] - time_list[0];
    double T = 1;
    for (size_t ord_idx = 0; ord_idx < order + 1; ++ord_idx) {
      poly(sample_idx, ord_idx) = T;
      T *= t;
    }
  }

  Eigen::MatrixXd res(3, order + 1);

  auto XtX = poly.transpose() * poly; // NxM MxN -> NxN (order + 1)^2
  auto XtXinv = XtX.inverse();
  auto XtXinvXt = XtXinv * poly.transpose();

  res.row(0) = (XtXinvXt * getPosVector(id, sample_count, 0)).transpose();
  res.row(1) = (XtXinvXt * getPosVector(id, sample_count, 1)).transpose();
  res.row(2) = (XtXinvXt * getPosVector(id, sample_count, 2)).transpose();

  return res;
}

Eigen::MatrixXd EFHOAW::Impl::getPosVector(size_t id, size_t sample_count, size_t dim) const {

  const position_list_t &position_list = history.at(id).second;
  Eigen::MatrixXd res(sample_count, 1);

  for (size_t sample_idx = 0; sample_idx < sample_count; ++sample_idx)
    res(sample_idx, 0) = position_list[sample_idx][dim];

  return res;
}

Eigen::Vector3d EFHOAW::getPolynomialPosition(int id, double t) const {
  return _impl->getPolynomialPosition(id, t);
}

Eigen::Vector3d EFHOAW::Impl::getPolynomialPosition(int id, double t) const {

  if (coefficients.find(id) == coefficients.end())
    throw gmCore::InvalidArgument("no coefficients calculated for specified id");

  const time_list_t &time_list = history.at(id).first;

  return getPolynomialPosition(coefficients.at(id), t - time_list[0]);
}

Eigen::Vector3d EFHOAW::Impl::getPolynomialPosition(polco coefficients, double t) {

  size_t order = coefficients.cols();

  double T = 1;
  Eigen::Vector3d res = Eigen::Vector3d::Zero();

  for (size_t idx = 0; idx < order; ++idx) {
    res[0] += T * coefficients(0, idx);
    res[1] += T * coefficients(1, idx);
    res[2] += T * coefficients(2, idx);
    T *= t;
  }

  return res;
}

Eigen::Vector3d EFHOAW::getPolynomialVelocity(int id, double t) const {
  return _impl->getPolynomialVelocity(id, t);
}

Eigen::Vector3d EFHOAW::Impl::getPolynomialVelocity(int id, double t) const {

  if (coefficients.find(id) == coefficients.end())
    throw gmCore::InvalidArgument("no coefficients calculated for specified id");

  const time_list_t &time_list = history.at(id).first;

  return getPolynomialVelocity(coefficients.at(id), t - time_list[0]);
}

Eigen::Vector3d EFHOAW::Impl::getPolynomialVelocity(polco coefficients, double t) {

  size_t order = coefficients.cols();

  double T = 1;
  Eigen::Vector3d res = Eigen::Vector3d::Zero();

  for (size_t idx = 1; idx < order; ++idx) {
    res[0] += idx * T * coefficients(0, idx);
    res[1] += idx * T * coefficients(1, idx);
    res[2] += idx * T * coefficients(2, idx);
    T *= t;
  }

  return res;
}

END_NAMESPACE_GMMISC;

#endif
