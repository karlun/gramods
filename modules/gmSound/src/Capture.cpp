
#include <gmSound/Capture.hh>

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>

#include <gmCore/Console.hh>

#include <optional>

namespace gramods {
namespace gmSound {

template<size_t N>
std::array<std::vector<std::int16_t>, N>
Capture::deinterlaceSamples(std::vector<std::int16_t> data) {
  if (data.size() % N)
    throw gmCore::InvalidArgument(
        "Number of samples not evenly dividable with number of channels");

  std::array<std::vector<std::int16_t>, N> results;
  for (size_t channel = 0; channel < N; ++channel) {
    auto &result = results[channel];
    result.reserve(data.size() / N);
    for (size_t idx = 0; idx < data.size(); idx += N)
      result.push_back(data[idx + channel]);
  }

  return results;
}

std::vector<std::vector<std::int16_t>>
Capture::deinterlaceSamples(std::vector<std::int16_t> data, size_t N) {
  if (data.size() % N)
    throw gmCore::InvalidArgument(
        "Number of samples not evenly dividable with number of channels");

  std::vector<std::vector<std::int16_t>> results(N);
  for (size_t channel = 0; channel < N; ++channel) {
    auto &result = results[channel];
    result.reserve(data.size() / N);
    for (size_t idx = 0; idx < data.size(); idx += N)
      result.push_back(data[idx + channel]);
  }

  return results;
}
}}
