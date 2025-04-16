
#include <gmSound/OpenALCapture.hh>

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>

#include <gmCore/Console.hh>

#include <optional>

namespace gramods {
namespace gmSound {

GM_OFI_DEFINE(OpenALCapture);
GM_OFI_PARAM2(OpenALCapture, deviceName, std::string, setDeviceName);
GM_OFI_PARAM2(OpenALCapture, sampleRate, size_t, setSampleRate);
GM_OFI_PARAM2(OpenALCapture, channelCount, size_t, setChannelCount);

struct OpenALCapture::Impl {
  ~Impl();

  void open();
  void setFormat(const char *name);

  std::optional<std::string> device_name;

  ALCdevice *device = nullptr;
  ALCuint sample_rate = 44100;
  ALCenum format = AL_FORMAT_MONO16;
  ALCsizei channels = 1;
  ALCsizei buffer_size = 4096;
};

OpenALCapture::OpenALCapture() : _impl(std::make_unique<Impl>()) {}
OpenALCapture::~OpenALCapture() {}

void OpenALCapture::initialize() {
  Capture::initialize();
  _impl->open();
}

void OpenALCapture::Impl::open() {

  if (!device_name)
    device_name = alcGetString(NULL, ALC_CAPTURE_DEFAULT_DEVICE_SPECIFIER);

  device = alcCaptureOpenDevice(
      device_name->c_str(), sample_rate, format, buffer_size);
  if (device) return;

  GM_ERR("OpenALCapture",
         "Could not open capture device '" << *device_name << "'!");
}

bool OpenALCapture::isOpen() { return _impl->device != nullptr; }

OpenALCapture::Impl::~Impl() {
  if (!device) return;
  alcCaptureStop(device);
  alcCaptureCloseDevice(device);
  device = nullptr;
}

void OpenALCapture::setDeviceName(std::string name) {
  _impl->device_name = name;
}

void OpenALCapture::setSampleRate(size_t f) {
  _impl->sample_rate = f;
}

size_t OpenALCapture::getSampleRate() { return _impl->sample_rate; }

void OpenALCapture::setChannelCount(size_t n) {
  switch (n) {
  case 1:
    _impl->format = AL_FORMAT_MONO16;
    _impl->channels = 1;
    return;
  case 2:
    _impl->format = AL_FORMAT_STEREO16;
    _impl->channels = 2;
    return;
  case 4:
    _impl->setFormat("AL_FORMAT_QUAD16");
    _impl->channels = 4;
    return;
  case 6:
    _impl->setFormat("AL_FORMAT_51CHN16");
    _impl->channels = 6;
    return;
  case 7:
    _impl->setFormat("AL_FORMAT_61CHN16");
    _impl->channels = 7;
    return;
  case 8:
    _impl->setFormat("AL_FORMAT_71CHN16");
    _impl->channels = 8;
    return;
  }
  GM_ERR("OpenALCapture", "Invalid channel count: " << n);
}

size_t OpenALCapture::getChannelCount() { return _impl->channels; }

void OpenALCapture::Impl::setFormat(const char *name) {
  ALenum fmt = alGetEnumValue(name);
  if (fmt) {
    format = fmt;
  } else {
    GM_ERR("OpenALCapture", "Format " << name << " not supported by device");
  }
}

std::vector<std::string> OpenALCapture::getCaptureDeviceNames() {

  std::vector<std::string> list;

  auto data = alcGetString(NULL, ALC_CAPTURE_DEVICE_SPECIFIER);
  while (true) {
    list.push_back(std::string(data));
    data += list.back().size() + 1;
    if (!data[0]) break;
  }
  return list;
}

void OpenALCapture::startCapture() {
  if (!_impl->device) return;
  alcCaptureStart(_impl->device);
}

void OpenALCapture::stopCapture() {
  if (!_impl->device) return;
  alcCaptureStop(_impl->device);
}

std::vector<float> OpenALCapture::getAvailableSamples() {
  if (!_impl->device) return {};

  ALCint count;
  alcGetIntegerv(_impl->device, ALC_CAPTURE_SAMPLES, 1, &count);

  if (count < 1) return {};

  std::vector<std::int16_t> bytes(count * _impl->channels, 0);
  alcCaptureSamples(_impl->device, bytes.data(), count);

  std::vector<float> samples;
  samples.reserve(bytes.size());
  for (auto val : bytes)
    samples.push_back(val / (float)std::numeric_limits<std::int16_t>::max());

  return samples;
}

size_t OpenALCapture::getAvailableSamplesCount() {
  if (!_impl->device) return 0;

  ALCint count;
  alcGetIntegerv(_impl->device, ALC_CAPTURE_SAMPLES, 1, &count);
  return count;
}

}}
