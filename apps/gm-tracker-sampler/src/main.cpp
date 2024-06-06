

#include <gmCore/Configuration.hh>
#include <gmCore/CommandLineParser.hh>
#include <gmCore/Updateable.hh>
#include <gmCore/Console.hh>
#include <gmCore/RunOnce.hh>
#include <gmCore/RuntimeException.hh>
#include <gmCore/TimeTools.hh>

#include <gmCore/io_eigen.hh>
#include <gmTrack/SampleCollector.hh>

#include <tclap/CmdLine.h>

#include <chrono>
#include <sstream>
#include <fstream>

using namespace gramods;

int main(int argc, char *argv[]) {

  TCLAP::CmdLine cmd
    ("This is a tracker sampler for gramods. A configuration file or string must be specified and this must contain the definition of a controller (gmTrack::Controller).");

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

  TCLAP::ValueArg<std::string> arg_outputfile
    ("o", "output-file",
     "The file to write the results to.",
     false, "output.csv", "file");
  cmd.add(arg_outputfile);

  TCLAP::ValueArg<float> arg_pos_inlier(
      "", "position-inlier-threshold",
      "Set the maximum positional distance from the average allowed for a"
      " sample to be included in the average. Default is to include all"
      " samples.",
      false, -1, "D");
  cmd.add(arg_pos_inlier);

  TCLAP::ValueArg<float> arg_ori_inlier(
      "", "orientation-inlier-threshold",
      "Set the maximum orientational distance (in radians) from the average"
      " allowed for a sample to be included in the average. Default is to"
      " include all samples.",
      false, -1, "D");
  cmd.add(arg_ori_inlier);

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

  std::shared_ptr<gmTrack::Controller> controller;
  if (!config->getObject(controller)) {
    std::cerr << "Error: No controller found." << std::endl;
    return 3;
  }

  std::shared_ptr<gmTrack::SampleCollector> collector =
      std::make_shared<gmTrack::SampleCollector>();
  collector->setController(controller);
  collector->setInlierThreshold(arg_pos_inlier.getValue());
  collector->setOrientationInlierThreshold(arg_ori_inlier.getValue());

  std::string output_file = arg_outputfile.getValue();
  std::ofstream fout(output_file);

  if (!fout || !(fout << "Time;Px;Py;Pz;Qx;Qy;Qz;Qw;Rx;Ry;Rz;Ra\n")) {
    std::cerr << std::endl
              << "Error: could not write to file '" << output_file << "'" << std::endl
              << std::endl;
    return 5;
  }

  typedef gmCore::TimeTools::clock clock;
  clock::time_point start_time = clock::now();

  size_t n_collected = 0;
  while (true) {
    gmCore::Updateable::updateAll();
    clock::time_point now_time = clock::now();

    auto & positions = collector->getTrackerPositions();
    auto & orientations = collector->getTrackerOrientations();

    if (positions.size() == n_collected) continue;

    double time_seconds = gmCore::TimeTools::durationToSeconds(now_time - start_time);
    auto & position = positions.back();
    auto & orientation = orientations.back();

    Eigen::AngleAxisf orientation_aa(orientation);

    fout << time_seconds << ";"
         << position[0] << ";"
         << position[1] << ";"
         << position[2] << ";"
         << orientation.x() << ";"
         << orientation.y() << ";"
         << orientation.z() << ";"
         << orientation.w() << ";"
         << orientation_aa.axis()[0] << ";"
         << orientation_aa.axis()[1] << ";"
         << orientation_aa.axis()[2] << ";"
         << orientation_aa.angle() << std::endl;

    ++n_collected;
  }

  return 0;
}
