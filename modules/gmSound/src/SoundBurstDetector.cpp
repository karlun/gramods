
#include <gmSound/SoundBurstDetector.hh>

#include <gmCore/Console.hh>

#include <optional>

namespace gramods {
namespace gmSound {

GM_OFI_DEFINE_SUB(SoundBurstDetector, SoundDetector);
GM_OFI_PARAM2(SoundBurstDetector, threshold, float, setThreshold);

struct SoundBurstDetector::Impl {

  std::vector<std::vector<float>>
  findDistances(const std::vector<std::deque<float>> &data,
                const float time_per_sample);

  float threshold = 0.7f;
  float cooldown_time = 0.1;
};

SoundBurstDetector::SoundBurstDetector() : _impl(std::make_unique<Impl>()) {}
SoundBurstDetector::~SoundBurstDetector() {}

void SoundBurstDetector::setThreshold(float r) {
  if (r <= 0.f || 1.f <= r)
    throw gmCore::InvalidArgument(
        GM_STR("Threshold (" << r << ") must be in range (0-1)"));
  _impl->threshold = r;
}

void SoundBurstDetector::setCooldown(float t) {
  if (t <= 0.f)
    throw gmCore::InvalidArgument(
        GM_STR("Cooldown (" << t << ") must be positive"));
  _impl->cooldown_time = t;
}

std::vector<std::vector<float>> SoundBurstDetector::detectSound() {
  const std::vector<std::deque<float>> &data = getData();
  const float time_per_sample = 1.f / getSampleRate();

  return _impl->findDistances(data, time_per_sample);
}

std::vector<std::vector<float>> SoundBurstDetector::Impl::findDistances(
    const std::vector<std::deque<float>> &data,
    const float time_per_sample) {

  size_t cooldown_samples = cooldown_time / time_per_sample;

  std::vector<std::vector<float>> result;
  size_t hit_count = 0;
  for (const auto &samples : data) {
    std::vector<float> hits;
    size_t next_valid_idx = 0;
    for (size_t idx = 0; idx < samples.size(); ++idx) {
      if (idx < next_valid_idx || std::abs(samples[idx]) < threshold) continue;
      hits.push_back(time_per_sample * (samples.size() - idx - 1));
      next_valid_idx = idx + cooldown_samples;
      ++hit_count;
      break;
    }
    result.push_back(hits);
  }

  GM_DBG2("SoundBurstDetector", "Detected time offsets: " << hit_count);

  return result;
}

}}
