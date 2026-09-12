

#include <gmCore/Configuration.hh>
#include <gmCore/CommandLineParser.hh>
#include <gmCore/Updateable.hh>
#include <gmCore/Console.hh>
#include <gmCore/RunOnce.hh>
#include <gmCore/RuntimeException.hh>
#include <gmCore/TimeTools.hh>

#include <gmCore/io_eigen.hh>

#include <gmTrack/TrackerBase.hh>
#include <gmTrack/TrackerSet.hh>

#include <tclap/CmdLine.h>

#include <chrono>
#include <fstream>
#include <optional>
#include <sstream>
#include <unordered_set>

#include <signal.h>

namespace {
bool alive = true;
void signal_handler(int s) {
  alive = false;
}
}

using namespace gramods;

namespace {
template <class TYPE>
void printSamples(const std::string &key,
                  std::unordered_map<std::string, float> &sample_time,
                  const TYPE &state, float T, float time_seconds) {
  if (!state) return;
  for (auto sample : state.value()) {
    if (sample_time.contains(sample.first) &&
        (T < 0 || time_seconds - sample_time.at(sample.first) < T))
      continue;
    std::cout << key << " '" << sample.first //
              << "' @ " << sample.second.value << std::endl;
    sample_time[sample.first] = time_seconds;
  }
}
}

int main(int argc, char *argv[]) {

  TCLAP::CmdLine cmd
    ("This is a tracker information printer for gramods. A configuration file or string must be specified and this must contain the definition of a tracker set (gmTrack::TrackerSet).");

  TCLAP::MultiArg<std::string> arg_config_dummy
    ("", "config",
     "Configuration file(s) load.",
     false, "file");
  TCLAP::MultiArg<std::string> arg_xml_dummy
    ("", "xml",
     "XML configuration(s) to load.",
     false, "string");
  cmd.xorAdd(arg_config_dummy, arg_xml_dummy);

  TCLAP::MultiArg<std::string> arg_param_dummy
    ("", "param",
     "Overrides configuration parameters. For example, if there is a node 'head' under the root and this node has a parameter 'connectionString' then this parameter can be overridden by '--param head.connectionString=WAND@localhost'.",
     false, "identifier=value");
  cmd.add(arg_param_dummy);

  TCLAP::ValueArg<float> delay(
      "d", "delay",
      "Delay between printing tracker sample value, in seconds, if available continuously. Default is -1, each tracker only printed once.",
      false, -1.f, "seconds");
  cmd.add(delay);

  try {
    cmd.parse(argc, argv);
  } catch (const TCLAP::ArgException &e) {
    std::cerr << "Error: " << e.error() << " for arg " << e.argId() << std::endl;
    return 1;
  }

  if (arg_config_dummy.getValue().empty() && arg_xml_dummy.getValue().empty())
    return 1;

  std::unique_ptr<gmCore::Configuration> config;
  try {
    config = std::make_unique<gmCore::Configuration>(argc, argv);
  } catch (const gmCore::RuntimeException &e) {
    std::cerr << "Error: Configuration error: " << e.what << std::endl;
  } catch (...) {
    std::cerr
        << "Error: Unknown internal error while creating Configuration instance."
        << std::endl;
  }

  if (!config) return 2;

  std::shared_ptr<gmTrack::TrackerSet> tracker_set;
  if (!config->getObject(tracker_set)) {
    std::cerr << "Error: No tracker set found." << std::endl;
    return 3;
  }

  float T = delay.getValue();

  signal(SIGINT, signal_handler);
  signal(SIGTERM, signal_handler);
#ifndef _WIN32
  signal(SIGQUIT, signal_handler);
#endif

  std::unordered_map<std::string, float> poses_time;
  std::unordered_map<std::string, float> binaries_time;
  std::unordered_map<std::string, float> floats_time;
  std::unordered_map<std::string, float> float2s_time;

  typedef gmCore::TimeTools::clock clock;
  clock::time_point start_time = clock::now();

  while (alive) {
    gmCore::Updateable::updateAll();
    clock::time_point now_time = clock::now();
    double time_seconds = gmCore::TimeTools::durationToSeconds(now_time - start_time);

    printSamples("Pose", poses_time, tracker_set->getPose(), //
                 T, time_seconds);
    printSamples("Binary", binaries_time, tracker_set->getBinary(), //
                 T, time_seconds);
    printSamples("Float", floats_time, tracker_set->getFloat(), //
                 T, time_seconds);
    printSamples("Float2", float2s_time, tracker_set->getFloat2(), //
                 T, time_seconds);
  }

  return 0;
}
