
#include <gmSound/SoundDetector.hh>
#include <gmSound/SoundConstants.hh>

#include <gmCore/Console.hh>
#include <gmCore/PreConditionViolation.hh>
#include <gmCore/RunOnce.hh>

namespace gramods {
namespace gmSound {

GM_OFI_DEFINE_ABSTRACT(SoundDetector);
GM_OFI_POINTER2(SoundDetector, capture, gmSound::Capture, setCapture);
GM_OFI_PARAM2(SoundDetector, window, float, setWindow);

struct SoundDetector::Impl {

  bool refreshData();

  std::shared_ptr<Capture> capture;

  float window = 10.f / SPEED_OF_SOUND_T20;

  std::vector<std::deque<float>> data;
};

SoundDetector::SoundDetector() : _impl(std::make_unique<Impl>()) {}
SoundDetector::~SoundDetector() {}

size_t SoundDetector::getSampleRate() {
  if (!_impl->capture) return 0;
  return _impl->capture->getSampleRate();
}

size_t SoundDetector::getChannelCount() {
  if (!_impl->capture) return 0;
  return _impl->capture->getChannelCount();
}

void SoundDetector::setCapture(std::shared_ptr<Capture> c) {
  if (!c) throw gmCore::InvalidArgument("Cannot use null capture device");
  if (_impl->capture) _impl->capture->stopCapture();
  _impl->capture = c;
  _impl->capture->startCapture();
}

void SoundDetector::setWindow(float t) {
  if (t <= std::numeric_limits<float>::epsilon())
    throw gmCore::InvalidArgument("too small window");
  _impl->window = t;
}

const std::vector<std::deque<float>> &SoundDetector::getData() {
  static std::vector<std::deque<float>> no_data;
  if (!_impl->refreshData()) return no_data;
  return _impl->data;
}

bool SoundDetector::Impl::refreshData() {
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

}}
