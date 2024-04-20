
#include <gmSound/Multilateration.hh>

#include <gmCore/eigen.hh>
#include <gmCore/Console.hh>
#include <gmCore/RunOnce.hh>
#include <gmCore/PreConditionViolation.hh>
#include <gmCore/TimeTools.hh>

#include <gmMisc/NelderMead.hh>

#include <optional>
#include <deque>

namespace gramods { namespace gmSound {

GM_OFI_DEFINE(Multilateration);
GM_OFI_PARAM2(Multilateration, point, Eigen::Vector3f, addPoint);
GM_OFI_PARAM2(Multilateration, threshold, float, setThreshold);
GM_OFI_PARAM2(Multilateration, window, float, setWindow);
GM_OFI_PARAM2(Multilateration, speedOfSound, float, setSpeedOfSound);
GM_OFI_POINTER2(Multilateration, capture, gmSound::Capture, setCapture);

struct Multilateration::Impl {
  void update(clock::time_point t);
  void initialize();

  bool refreshData();
  std::optional<std::vector<float>> findDistances();
  void estimatePose(std::vector<float> &distances,
                    clock::time_point time);

  std::optional<PoseSample> pose;

  std::vector<Eigen::Vector3f> points;
  float max_mic_dist = 0;

  float threshold = 0.7;
  float window = 0.05;
  float speed_of_sound = 343;
  std::shared_ptr<Capture> capture;

  std::vector<std::deque<std::int16_t>> data;
};

Multilateration::Multilateration()
  : gmCore::Updateable(10), _impl(std::make_unique<Impl>()) {}
Multilateration::~Multilateration() {}

void Multilateration::addPoint(Eigen::Vector3f pt) {
  _impl->points.push_back(pt);
}

void Multilateration::setThreshold(float r) { _impl->threshold = r; }

void Multilateration::setWindow(float t) { _impl->window = t; }

void Multilateration::setSpeedOfSound(float v) { _impl->speed_of_sound = v; }

void Multilateration::setCapture(std::shared_ptr<Capture> c) {
  _impl->capture = c;
}

bool Multilateration::getPose(PoseSample &p) {
  if (!_impl->pose) return false;

  p = *_impl->pose;
  return true;
}

void Multilateration::update(clock::time_point t, size_t frame) {
  _impl->update(t);
}

void Multilateration::initialize() {
  _impl->initialize();
  if (_impl->capture) _impl->capture->startCapture();
}

void Multilateration::Impl::initialize() {
  max_mic_dist = 0.f;
  for (const auto &pt1 : points)
    for (const auto &pt2 : points)
      max_mic_dist = std::max(max_mic_dist, (pt1 - pt2).norm());
}

void Multilateration::Impl::update(clock::time_point time) {
  if (!capture) {
    GM_RUNONCE(GM_ERR(
        "Multilateration",
        "Cannot read data without a Capture instance to read data from!"));
    return;
  }

  if (!refreshData()) return;
  auto distances = findDistances();
  if (distances) estimatePose(*distances, time);
}

bool Multilateration::Impl::refreshData() {
  const auto channels = capture->getChannelCount();
  if (channels < 1)
    throw gmCore::PreConditionViolation("Too few audio channels");

  if (!data.size()) data.resize(channels);
  else if (data.size() != channels) {
    GM_RUNONCE(GM_ERR("Multilateration", "Channel count changed - cannot proceed!"));
    return false;
  }

  if (capture->getAvailableSamplesCount() < 1) return false;

  const auto interlaced_samples = capture->getAvailableSamples();
  const auto samples =
      Capture::deinterlaceSamples(interlaced_samples, channels);

  for (size_t idx = 0; idx < channels; ++idx)
    data[idx].insert(data[idx].end(), samples[idx].begin(), samples[idx].end());

  const size_t max_samples = size_t(window * capture->getSampleRate());
  if (data[0].size() < max_samples) return true;
  // If there is too much data, remove some

  const auto rmN = data[0].size() - max_samples;
  for (size_t idx = 0; idx < channels; ++idx)
    data[idx].erase(data[idx].begin(), data[idx].begin() + rmN);

  return true;
}

std::optional<std::vector<float>>
Multilateration::Impl::findDistances() {

  const std::int16_t level =
      std::int16_t(threshold * std::numeric_limits<std::int16_t>::max());

  std::vector<float> result;
  for (const auto &samples : data) {
    std::optional<size_t> hit_idx;
    for (size_t idx = 0; idx < samples.size(); ++idx) {
      if (samples[idx] < level && samples[idx] > -level) continue;
      hit_idx = idx;
      break;
    }
    if (hit_idx) result.push_back(*hit_idx);
  }

  if (result.size() != data.size()) return std::nullopt;

  const float m_per_sample = speed_of_sound / capture->getSampleRate();
  for (auto &value : result) value = m_per_sample * value;

  std::stringstream dbg_str;
  for (const auto d : result) dbg_str << d << " ";
  GM_DBG2("Multilateration", "Detected distances: " << dbg_str.str());

  return result;
}

void Multilateration::Impl::estimatePose(std::vector<float> &distances,
                                         clock::time_point time) {
  float d0 = std::numeric_limits<float>::max();
  for (const auto value : distances) d0 = std::min(d0, value);
  for (auto &value : distances) value -= d0;

  std::vector<Eigen::Vector4f> x0;
  for (const auto &pt : points)
    x0.push_back({pt[0], pt[1], pt[2], 0.2f * max_mic_dist});
  {
    auto const &pt = points.back();
    x0.push_back({pt[0], pt[1], pt[2], 0.8f * max_mic_dist});
  }

  size_t iterations = 0;

  auto res4 = gmMisc::NelderMead::solve<float>(
      x0,
      [&, this](const Eigen::Vector4f &x) {
        Eigen::Vector3f p = x.block<3, 1>(0, 0);
        float d0 = x[3];

        float err2 = 0.f;
        for (size_t idx = 0; idx < points.size(); ++idx) {
          float err = (points[idx] - p).norm() - (distances[idx] + d0);
          err2 += err * err;
        }
        return err2;
      },
      iterations);

  const auto pt = res4.block<3, 1>(0, 0);
  const auto d1 = res4[3];

  float window_secs = data[0].size() / float(capture->getSampleRate());
  auto dt = -window_secs + (d0 - d1) / speed_of_sound;
  GM_DBG2("Multilateration", "Time offset: " << dt << " seconds");

  pose = {pt,
          Eigen::Quaternionf::Identity(),
          time + gmCore::TimeTools::secondsToDuration(dt)};
}
}}
