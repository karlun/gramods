
#include <gmSound/OpenALCapture.hh>
#include <gmCore/TimeTools.hh>

#include <tclap/CmdLine.h>

#include <chrono>
#include <fstream>

using namespace gramods;

int main(int argc, char *argv[]) {

  TCLAP::CmdLine cmd
    ("This is a test of the audio capture functionality in gramods.");

  TCLAP::SwitchArg arg_list_devices
    ("", "list-devices",
     "Lists the currently available audio capture devices.",
     cmd, false);
  TCLAP::ValueArg<std::string> arg_device_name(
      "d", "device", "Select device to open by name.", false, "", "NAME", cmd);
  TCLAP::ValueArg<size_t> arg_device_idx(
      "D", "device-idx", "Select device to open by index.", false, 0, "idx", cmd);
  TCLAP::ValueArg<size_t> arg_channels(
      "c", "channels", "Number of channels to open (default 2).", false, 2, "idx", cmd);
  TCLAP::ValueArg<float> arg_time(
      "t", "time", "Time (in seconds) to keep reading data (default 10).", false, 10, "sec", cmd);
  TCLAP::ValueArg<std::string> arg_filename(
      "o", "output", "File to save audio data to. The data are saved in ascii, one row per sample.", false, "", "path", cmd);

  try {
    cmd.parse(argc, argv);
  } catch (const TCLAP::ArgException &e) {
    std::cerr << "Error: " << e.error() << " for arg " << e.argId() << "\n";
    return 1;
  }

  if (arg_device_name.isSet() && arg_device_idx.isSet()) {
    std::cerr << "Device can only be selected either by index or name, not both at the same time\n";
    return -1;
  }

  auto devices = gmSound::OpenALCapture::getCaptureDeviceNames();
  if (arg_list_devices.getValue()) {
    for (size_t idx = 0; idx < devices.size(); ++idx)
      std::cout << idx << ": " << devices[idx] << "\n";
    return 0;
  }

  gmSound::OpenALCapture capture;
  if (arg_device_idx.isSet()) {
    capture.setDeviceName(devices[arg_device_idx.getValue()]);
  } else if (arg_device_idx.isSet()) {
    capture.setDeviceName(arg_device_name.getValue());
  }

  capture.setChannelCount(arg_channels.getValue());
  capture.initialize();

  if (!capture.isOpen()) return -2;

  float max_value = std::numeric_limits<float>::min();
  float min_value = std::numeric_limits<float>::max();

  capture.startCapture();

  std::ofstream file;

  if (arg_filename.isSet()) file.open(arg_filename.getValue());

  auto start_time = std::chrono::steady_clock::now();
  while (std::chrono::steady_clock::now() - start_time <
         gmCore::TimeTools::secondsToDuration(arg_time.getValue())) {
    size_t count = capture.getAvailableSamplesCount();
    if (count < 512) continue;

    auto samples_data = capture.getAvailableSamples();
    auto channel_data = gmSound::Capture::deinterlaceSamples(
        samples_data, capture.getChannelCount());

    if (file)
      for (size_t sp = 0; sp < channel_data[0].size(); ++sp) {
        for (size_t ch = 0; ch < channel_data.size(); ++ch)
          file << channel_data[ch][sp] << " ";
        file << "\n";
      }

    float current_max_value = std::numeric_limits<float>::min();
    float current_min_value = std::numeric_limits<float>::max();
    for (auto val : samples_data) {
      current_max_value = std::max(val, current_max_value);
      current_min_value = std::min(val, current_min_value);
    }

    if (current_min_value < min_value || current_max_value > max_value) {
      min_value = std::min(min_value, current_min_value);
      max_value = std::max(max_value, current_max_value);
      std::cerr << "Span: " << min_value << " - " << max_value << "\n";
    }
  }
  capture.stopCapture();

  return 0;
}
