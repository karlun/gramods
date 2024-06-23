
#include <gmSound/CaptureFromWav.hh>

#include <gmCore/Console.hh>
#include <gmCore/ExitException.hh>
#include <gmCore/TimeTools.hh>
#include <gmCore/PreConditionViolation.hh>
#include <gmCore/UnsupportedException.hh>

#include <bit>
#include <fstream>
#include <optional>

namespace gramods::gmSound {

GM_OFI_DEFINE(CaptureFromWav);
GM_OFI_PARAM2(CaptureFromWav, file, std::filesystem::path, setFile);
GM_OFI_PARAM2(CaptureFromWav, loop, bool, setLoop);
GM_OFI_PARAM2(CaptureFromWav, exit, bool, setExit);

#define STR(X, N) std::string(X, X + N);

struct CaptureFromWav::Impl {

  struct WaveChunkHeader {
    std::string str() { return std::string(id, id + 4); }
    char id[4];
    std::uint32_t size;
  };

  struct RiffHeader {
    std::string str() { return std::string(format, format + 4); }
    char format[4];
  };

  struct WaveFormat {
    std::int16_t audioFormat;
    std::int16_t numChannels;
    std::int32_t sampleRate;
    std::int32_t byteRate;
    std::int16_t blockAlign;
    std::int16_t bitsPerSample;
  };

  struct Data {
    Data(size_t rate, size_t channels, size_t count)
      : sample_rate(rate),
        channel_count(channels),
        samples(std::vector<float>(count, 0)) {}

    const size_t sample_rate;
    const size_t channel_count;
    std::vector<float> samples;
    size_t ptr = 0;
  };

  void loadWavFile(std::filesystem::path file);
  std::vector<float> getAvailableSamples();
  size_t getAvailableSamplesCount();

  typedef gmCore::TimeTools::clock clock;

  std::optional<std::filesystem::path> file_path;
  std::optional<Data> data;
  std::optional<clock::time_point> last_touch;

  bool do_loop = false;
  bool do_exit = true;
};

CaptureFromWav::CaptureFromWav() : _impl(std::make_unique<Impl>()) {
  if constexpr (std::endian::native != std::endian::little)
    throw gmCore::UnsupportedException(
        "WAV reader does not perform byte swapping and thus requires some extra"
        " work to support non-little endian platforms");
}
CaptureFromWav::~CaptureFromWav() {}

void CaptureFromWav::setFile(std::filesystem::path file) {
  if (isInitialized())
    throw gmCore::PreConditionViolation(
        "Cannot change file after initialization");
  _impl->file_path = file;
}

void CaptureFromWav::setLoop(bool on) {
  _impl->do_loop = on;
}

void CaptureFromWav::setExit(bool on) {
  _impl->do_exit = on;
}

size_t CaptureFromWav::getSampleRate() {
  if (!_impl->data)
    throw gmCore::PreConditionViolation("No sample rate available");
  return _impl->data->sample_rate;
}
size_t CaptureFromWav::getChannelCount() {
  if (!_impl->data)
    throw gmCore::PreConditionViolation("No channel count available");
  return _impl->data->channel_count;
}

bool CaptureFromWav::isOpen() {
  return _impl->data && _impl->data->ptr < _impl->data->samples.size();
}

void CaptureFromWav::startCapture() { _impl->last_touch = Impl::clock::now(); }

void CaptureFromWav::stopCapture() { _impl->last_touch = std::nullopt; }

std::vector<float> CaptureFromWav::getAvailableSamples() {
  return _impl->getAvailableSamples();
}

std::vector<float> CaptureFromWav::Impl::getAvailableSamples() {
  if (!data || !last_touch) return {};

  if (data->ptr >= data->samples.size()) {
    if (do_exit) throw gmCore::ExitException(0);

    GM_WRN("CaptureFromWav", "Out of data");
    data->ptr = 0;
    last_touch = std::nullopt;
    return {};
  }

  auto now = clock::now();
  const auto delta_t =
      gmCore::TimeTools::durationToSeconds(now - *last_touch);
  last_touch = now;

  auto count = data->channel_count * size_t(data->sample_rate * delta_t);

  if (data->ptr + count < data->samples.size()) {
    std::vector<float> res(data->samples.begin() + data->ptr,
                                  data->samples.begin() + data->ptr + count);
    data->ptr += count;
    return res;
  } else if (!do_loop) {
    std::vector<float> res(data->samples.begin() + data->ptr,
                                  data->samples.end());
    data->ptr = data->samples.size();
    return res;
  } else {
    std::vector<float> res;
    res.reserve(count);
    size_t load, total = 0;
    do {
      size_t load = std::min(count, data->samples.size() - data->ptr);
      res.insert(res.end(),
                 data->samples.begin() + data->ptr,
                 data->samples.begin() + data->ptr + load);
      data->ptr = (data->ptr + load) % data->samples.size();
    } while ((total = total + load) < count);
    return res;
  }
}

size_t CaptureFromWav::getAvailableSamplesCount() {
  return _impl->getAvailableSamplesCount();
}

size_t CaptureFromWav::Impl::getAvailableSamplesCount() {
  if (!data || !last_touch) return 0;

  if (data->ptr >= data->samples.size()) {
    if (do_exit) throw gmCore::ExitException(0);

    data->ptr = 0;

    if (!do_loop) {
      GM_WRN("CaptureFromWav", "Out of data");
      last_touch = std::nullopt;
      return 0;
    }
  }

  const auto delta_t =
      gmCore::TimeTools::durationToSeconds(clock::now() - *last_touch);
  return do_loop ? size_t(data->sample_rate * delta_t)
                 : std::min(data->samples.size() - data->ptr,
                            size_t(data->sample_rate * delta_t));
}

void CaptureFromWav::initialize() {
  Capture::initialize();
  if (!_impl->file_path) {
    GM_ERR("CaptureFromWav", "No file specified");
    return;
  }

  _impl->loadWavFile(*_impl->file_path);
}

void CaptureFromWav::Impl::loadWavFile(std::filesystem::path file) {

  std::ifstream file_stream(file, std::ios::binary);
  if (!file_stream) {
    GM_ERR("CaptureFromWav", "Could not open file: " << file);
    return;
  }

  WaveChunkHeader chunk_header;
  file_stream.read(reinterpret_cast<char *>(&chunk_header),
                   sizeof(WaveChunkHeader));
  if (chunk_header.str() != "RIFF" && chunk_header.str() != "WAVE") {
    GM_ERR("CaptureFromWav", "File not identified as RIFF or WAVE: " << file);
    return;
  }

  RiffHeader riff_header;
  file_stream.read(reinterpret_cast<char *>(&riff_header), sizeof(RiffHeader));
  GM_DBG2("CaptureFromWav", "RIFF format: " << riff_header.str());

  file_stream.read(reinterpret_cast<char *>(&chunk_header),
                   sizeof(WaveChunkHeader));
  if (chunk_header.str() != "fmt ") {
    GM_ERR("CaptureFromWav",
           "File corrupt (expected fmt chunk, got '" << chunk_header.str()
                                                     << "') in " << file);
    return;
  }
  if (chunk_header.size < sizeof(WaveFormat)) {
    GM_ERR("CaptureFromWav",
           "Incorrect WAVE format size (" << chunk_header.size << ") in "
                                          << file);
    return;
  }

  WaveFormat format;
  file_stream.read(reinterpret_cast<char *>(&format), sizeof(WaveFormat));
  file_stream.ignore(chunk_header.size - sizeof(WaveFormat));
  if (chunk_header.size > sizeof(WaveFormat))
    GM_DBG2("CaptureFromWav",
            "Ignored " << chunk_header.size - sizeof(WaveFormat)
                       << " bytes of unknown format data");

  while (file_stream.read(reinterpret_cast<char *>(&chunk_header),
                          sizeof(WaveChunkHeader)) &&
         chunk_header.str() != "data") {
    file_stream.ignore(chunk_header.size);
    GM_DBG2("CaptureFromWav",
            "Ignored " << chunk_header.size << " bytes of unknown data ('"
                       << chunk_header.str() << "')");
  }

  if (!file_stream) {
    GM_ERR("CaptureFromWav",
           "IO error while searching for data chunk in " << file);
    return;
  }

  std::vector<char> samples(chunk_header.size, 0);
  if (!file_stream.read(samples.data(), chunk_header.size)) {
    GM_ERR("CaptureFromWav", "IO error while reading data chunk in " << file);
    return;
  }

  if (format.bitsPerSample == 8 && format.audioFormat == 1) {
    data.emplace(format.sampleRate, format.numChannels, chunk_header.size);
    for (size_t idx = 0; idx < chunk_header.size; ++idx)
      data->samples[idx] =
          -1.f + (2.f / std::numeric_limits<std::uint8_t>::max()) *
                      reinterpret_cast<std::uint8_t *>(samples.data())[idx];

  } else if (format.bitsPerSample == 16 && format.audioFormat == 1) {
    data.emplace(format.sampleRate, format.numChannels, chunk_header.size / 2);
    for (size_t idx = 0; idx < chunk_header.size / 2; ++idx)
      data->samples[idx] =
          (1.f / std::numeric_limits<std::int16_t>::max()) *
          reinterpret_cast<std::int16_t *>(samples.data())[idx];

  } else if (format.bitsPerSample == 24 && format.audioFormat == 1) {
    data.emplace(format.sampleRate, format.numChannels, chunk_header.size / 3);
    for (size_t idx = 0; idx < chunk_header.size / 3; ++idx)
      // FIX THIS.
      data->samples[idx] = (1.f / std::numeric_limits<char>::max()) *
                           samples.data()[3 * idx + 2];

  } else if (format.bitsPerSample == 32 && format.audioFormat == 1) {
    data.emplace(format.sampleRate, format.numChannels, chunk_header.size / 4);
    for (size_t idx = 0; idx < chunk_header.size / 4; ++idx)
      data->samples[idx] =
          (1.f / std::numeric_limits<std::int32_t>::max()) *
          reinterpret_cast<std::int32_t *>(samples.data())[idx];

  } else if (format.bitsPerSample == 32 && format.audioFormat == 3) {
    data.emplace(format.sampleRate, format.numChannels, chunk_header.size / 4);
    for (size_t idx = 0; idx < chunk_header.size / 4; ++idx)
      data->samples[idx] = reinterpret_cast<float *>(samples.data())[idx];

  } else if (format.bitsPerSample == 64 && format.audioFormat == 3) {
    data.emplace(format.sampleRate, format.numChannels, chunk_header.size / 8);
    for (size_t idx = 0; idx < chunk_header.size / 8; ++idx)
      data->samples[idx] =
          float(reinterpret_cast<double *>(samples.data())[idx]);

  } else {
    GM_ERR("CaptureFromWav",
           "Unsupported format " << (format.audioFormat == 1   ? "integer"
                                     : format.audioFormat == 3 ? "float"
                                                               : "(unknown)")
                                 << " with " << format.bitsPerSample
                                 << " bits per sample in " << file);
  }
}
}
