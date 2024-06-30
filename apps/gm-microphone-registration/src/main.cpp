
#include <gmCore/Configuration.hh>
#include <gmCore/CommandLineParser.hh>
#include <gmCore/Console.hh>
#include <gmCore/RunOnce.hh>
#include <gmCore/io_eigen.hh>

#include <gmSound/SoundDetector.hh>
#include <gmSound/SoundConstants.hh>

#include <gmMisc/NelderMead.hh>

#include <tclap/CmdLine.h>
#include <Eigen/Eigen>

#include <chrono>
#include <sstream>
#include <fstream>
#include <optional>
#include <numeric>

using namespace gramods;

void write_data(std::vector<Eigen::Vector3f> points,
                std::ostream &out);

int collect_sound(std::vector<std::vector<float>> &collected_offsets,
                  std::shared_ptr<gmSound::SoundDetector> detector,
                  size_t microphone_count,
                  size_t sample_count);

int simulate_collection(
    const std::vector<std::string> &arg_simulated_mic_position,
    const std::vector<std::string> &arg_simulated_capture_position,
    float speed_of_sound,
    std::vector<std::vector<float>> &collected_offsets);

int main(int argc, char *argv[]) {

  TCLAP::CmdLine cmd
    ("This tools help registrering the relative position of microphones by estimating sound travel time between them. A configuration file or string must be specified and this must contain the definition of a sound detector (gmSound::SoundDetector). Play the sound on the extended line for each pair of microphones. The first microphone will be placed in the origin, the second along the x-axis, the third in the x-y plane (positive y) and the fourth in positive z.");

  TCLAP::MultiArg<std::string> arg_config_dummy
    ("", "config",
     "Configuration file(s) load.",
     false, "file", cmd);
  TCLAP::MultiArg<std::string> arg_xml_dummy
    ("", "xml",
     "XML configuration(s) to load.",
     false, "string", cmd);

  TCLAP::MultiArg<std::string> arg_param_dummy
    ("", "param",
     "Overrides configuration parameters. For example, if there is a node 'head' under the root and this node has a parameter 'connectionString' then this parameter can be overridden by '--param head.connectionString=WAND@localhost'.",
     false, "identifier=value", cmd);

  TCLAP::ValueArg<std::string> arg_outputfile
    ("o", "output-file",
     "The file to write the results to.",
     false, "output.txt", "file", cmd);

  TCLAP::ValueArg<size_t> arg_sample_count(
      "c", "sample-count",
      "Number of sound samples to collect. Default is 6, which is the optimal for a tetraedic microphone setup. Optimal is always N(N-1)/2 where N is the number of microphones.",
      false, 6, "N", cmd);
  TCLAP::ValueArg<size_t> arg_microphone_count(
      "m", "microphone-count",
      "Number of microphones to position. Default is 4, which is the minimum necessary for 3D multilateration.",
      false, 4, "N", cmd);

  TCLAP::ValueArg<float> arg_speed_of_sound(
      "", "speed-of-sound",
      "Sets the speed of sound to use when estimating microphone position.",
      false, -1, "c", cmd);
  TCLAP::ValueArg<float> arg_air_temperature(
      "", "air-temperature",
      "Sets the air temperature, to estimate speed of sound from. Default is 20°C",
      false, 20, "T", cmd);

  TCLAP::MultiArg<std::string> arg_simulated_mic_position
    ("", "simulated-microphone-position",
     "Simulates microphone position, to test algorithm.",
     false, "x,y,z", cmd);
  TCLAP::MultiArg<std::string> arg_simulated_capture_position
    ("", "simulated-capture-position",
     "Simulates sound capture position, to test algorithm.",
     false, "x,y,z", cmd);

  try {
    cmd.parse(argc, argv);
  } catch (const TCLAP::ArgException &e) {
    std::cerr << "Error: " << e.error() << " for arg " << e.argId() << "!\n";
    return 1;
  }

  std::unique_ptr<gmCore::Configuration> config;
  if (!arg_simulated_mic_position.isSet() &&
      !arg_simulated_capture_position.isSet())
    try {
      config = std::make_unique<gmCore::Configuration>(argc, argv);
    } catch (const gmCore::RuntimeException &e) {
      std::cerr << "Error: Configuration error: " << e.what << "!\n";
    } catch (...) {
      std::cerr
          << "Error: Unknown internal error while creating Configuration instance!\n";
    }

  std::shared_ptr<gmSound::SoundDetector> detector;
  if (!(config && config->getObject(detector)) &&
      !arg_simulated_mic_position.isSet() &&
      !arg_simulated_capture_position.isSet()) {
    std::cerr << "Error: No sound detector found!\n";
    return 3;
  }

  if (arg_microphone_count.getValue() < 2) {
    std::cerr
        << "Error: Cannot perform registration of less than two microphones!\n";
    return 4;
  }

  if (arg_speed_of_sound.isSet() && arg_air_temperature.isSet()) {
    std::cerr
        << "Error: Cannot use both specified speed of sound and calculate it from air temperature!\n";
    return 5;
  }
  float speed_of_sound =
      arg_speed_of_sound.isSet()
          ? arg_speed_of_sound.getValue()
          : SPEED_OF_SOUND.getValue(arg_air_temperature.getValue());

  std::optional<std::ofstream> file_out;
  if (arg_outputfile.isSet()) {
    file_out = std::ofstream(arg_outputfile.getValue());
    if (!*file_out) {
      std::cerr << "Could not open file '" << arg_outputfile.getValue()
                << "' for writing\n";
      return 6;
    }
  }

  std::vector<std::vector<float>> collected_offsets;
  if (arg_simulated_mic_position.isSet() !=
      arg_simulated_capture_position.isSet()) {
    std::cerr
        << "To simulate algorithm both microphone and capture positions need to be set!\n";
    return 7;
  } else if (arg_simulated_mic_position.isSet() &&
             arg_simulated_capture_position.isSet()) {
    int res = simulate_collection(arg_simulated_mic_position.getValue(),
                                  arg_simulated_capture_position.getValue(),
                                  speed_of_sound,
                                  collected_offsets);
    if (res) return res;
  } else {
    int res = collect_sound(collected_offsets,
                            detector,
                            arg_microphone_count.getValue(),
                            arg_sample_count.getValue());
    if (res) return res;
  }

  std::map<std::pair<size_t, size_t>, float> mic_pair_offset;
  float max_mic_offset = std::numeric_limits<float>::min();
  for (size_t idx0 = 0; idx0 < arg_microphone_count.getValue(); ++idx0)
    for (size_t idx1 = idx0 + 1; idx1 < arg_microphone_count.getValue(); ++idx1) {

      float best_distance =
          speed_of_sound * std::fabs(collected_offsets[0][idx0] - //
                                     collected_offsets[0][idx1]);
      size_t best_idx = 0;

      // Find largest distance
      for (size_t idx = 1; idx < collected_offsets.size(); ++idx) {
        float distance =
            speed_of_sound * std::fabs(collected_offsets[idx][idx0] - //
                                       collected_offsets[idx][idx1]);
        if (best_distance > distance) continue;
        best_distance = distance;
        best_idx = idx;
      }

      mic_pair_offset.insert({{idx0, idx1}, best_distance});
      max_mic_offset = std::max(max_mic_offset, best_distance);
    }

  std::vector<Eigen::Vector3f> points;
  points.push_back(Eigen::Vector3f::Zero());
  points.push_back(mic_pair_offset[{0, 1}] * Eigen::Vector3f::UnitX());

  if (arg_microphone_count.getValue() > 2) {
    float d = mic_pair_offset[{0, 1}];
    float R = mic_pair_offset[{0, 2}];
    float r = mic_pair_offset[{1, 2}];
    float x = (d * d + R * R - r * r) / (2.f * d);
    float y = std::sqrt(R * R - x * x);
    points.push_back(Eigen::Vector3f(x, y, 0));
  }

  Eigen::Vector3f mid_012 = (1.f / 3.f) * (points[0] + points[1] + points[2]);

  // An educated guess, or at least a simplex spanning the 3D space
  float z_offset = std::sqrt(2.f) * max_mic_offset;
  const std::vector<Eigen::Vector3f> X0 = {
      points[0] + 0.5f * z_offset * Eigen::Vector3f::UnitZ(),
      points[1] + 0.5f * z_offset * Eigen::Vector3f::UnitZ(),
      points[2] + 0.5f * z_offset * Eigen::Vector3f::UnitZ(),
      mid_012 + 1.5f * z_offset * Eigen::Vector3f::UnitZ()};

  for (size_t point_idx = 3; point_idx < arg_microphone_count.getValue();
       ++point_idx) {
    size_t iterations = 0;
    auto res = gmMisc::NelderMead::solve<float>(
        X0, [points, point_idx, mic_pair_offset](const Eigen::Vector3f &x) {
          float err2 = 0.f;
          for (size_t idx = 0; idx < points.size(); ++idx) {
            float err =
                (points[idx] - x).norm() - mic_pair_offset.at({idx, point_idx});
            err2 += err * err;
          }
          return err2;
        }, iterations);
    points.push_back(res);
  }

  std::cout << "Done! Estimated " << points.size()
            << " microphone positions.\n";
  if (file_out) {
    write_data(points, *file_out);
    if (*file_out)
      std::cout << "Results written to " << arg_outputfile.getValue() << "\n";
    else
      std::cout << "Failed to write results!\n";
  } else {
    std::cout << "\nResults:\n";
    write_data(points, std::cout);
    std::cout << "\n";
  }

  return 0;
}

void write_data(std::vector<Eigen::Vector3f> points,
                std::ostream &out) {
  for (const auto &pt : points)
    out << pt[0] << " " << pt[1] << " " << pt[2] << "\n";
  out << "\n";
}

int collect_sound(std::vector<std::vector<float>> &collected_offsets,
                  std::shared_ptr<gmSound::SoundDetector> detector,
                  size_t microphone_count,
                  size_t sample_count) {

  typedef std::chrono::steady_clock clock;
  typedef std::chrono::duration<double, std::ratio<1>> d_seconds;

  auto cooldown_timepoint = clock::now() + d_seconds(1);

  std::cout << "Collecting sound\n";

  while (true) {

    const auto &offsets = detector->detectSound();

    auto now = clock::now();
    if (now < cooldown_timepoint) continue;

    if (offsets.empty()) continue;
    if (offsets.size() < microphone_count) {
      std::cerr << "Invalid channel count: " << offsets.size() << " < "
                << microphone_count << "\n";
      return 7;
    }

    std::vector<float> first_offsets;
    first_offsets.reserve(offsets.size());
    for (const auto &co : offsets) {
      if (co.empty()) continue;
      first_offsets.push_back(co.front());
    }

    if (first_offsets.size() != offsets.size()) continue;
    first_offsets.erase(first_offsets.begin() + microphone_count,
                        first_offsets.end());

    collected_offsets.push_back(first_offsets);
    cooldown_timepoint = now + d_seconds(1);
    if (collected_offsets.size() < sample_count) {
      std::cout << "Collecting " << (collected_offsets.size() + 1) << " of "
                << sample_count << "\n";
    } else {
      break;
    }
  }

  return 0;
}

std::optional<Eigen::Vector3f> parse(std::string pt) {
  std::replace(pt.begin(), pt.end(), ',', ' ');
  std::stringstream ss(pt);
  Eigen::Vector3f p;
  if (ss >> p) return p;
  std::cerr << "Warning: could not parse '" << pt << "' as a 3D point.\n";
  return std::nullopt;
}

int simulate_collection(
    const std::vector<std::string> &arg_simulated_mic_position,
    const std::vector<std::string> &arg_simulated_capture_position,
    float speed_of_sound,
    std::vector<std::vector<float>> &collected_offsets) {

  std::vector<Eigen::Vector3f> simulated_mic_position;
  for (const auto & pt : arg_simulated_mic_position) {
    if (auto e3 = parse(pt)) simulated_mic_position.push_back(*e3);
  }

  std::vector<Eigen::Vector3f> simulated_capture_position;
  for (const auto & pt : arg_simulated_capture_position) {
    if (auto e3 = parse(pt)) simulated_capture_position.push_back(*e3);
  }

  for (const auto &cap_pos : simulated_capture_position) {
    std::vector<float> offsets;
    for (const auto &mic_pos : simulated_mic_position)
      offsets.push_back((cap_pos - mic_pos).norm() / speed_of_sound);
    float max_offset = std::reduce(
        offsets.begin(),
        offsets.end(),
        std::numeric_limits<float>::min(),
        [](const float &a, const float &b) { return std::max(a, b); });
    for (auto &o : offsets) o = 1.1f * max_offset - o;
    collected_offsets.push_back(offsets);
  }

  return 0;
}
