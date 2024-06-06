

#include <gmCore/Configuration.hh>
#include <gmCore/CommandLineParser.hh>
#include <gmCore/Updateable.hh>
#include <gmCore/Console.hh>
#include <gmCore/RunOnce.hh>
#include <gmCore/RuntimeException.hh>
#include <gmCore/TimeTools.hh>

#include <gmCore/io_eigen.hh>
#include <gmTrack/ProjectionTextureGenerator.hh>

#include <tclap/CmdLine.h>

#include <chrono>
#include <sstream>
#include <fstream>

using namespace gramods;

int main(int argc, char *argv[]) {

  TCLAP::CmdLine cmd(
      "This is a projection registration tool for gramods. A configuration file or string must be specified and this must contain the definition of a controller (gmTrack::Controller). To simplify registration it is preferred to also load a configuration file that provide visual indications of the registration points.");

  TCLAP::MultiArg<std::string> arg_config_dummy(
      "", "config", "Configuration file(s) load.", false, "file");
  TCLAP::MultiArg<std::string> arg_xml_dummy(
      "", "xml", "XML configuration(s) to load.", false, "string");
  cmd.xorAdd(arg_config_dummy, arg_xml_dummy);

  TCLAP::MultiArg<std::string> arg_param_dummy(
      "",
      "param",
      "Overrides configuration parameters. For example, if there is a node 'head' under the root and this node has a parameter 'connectionString' then this parameter can be overridden by '--param head.connectionString=WAND@localhost'.",
      false,
      "identifier=value");
  cmd.add(arg_param_dummy);

  TCLAP::MultiArg<std::string> arg_bpoint(
      "p",
      "b-point",
      "Specification of a point in the coordinate system of the projected buffer as x,y ranging 0-1 for left to right and bottom to top, respectively.",
      true,
      "x,y");
  cmd.add(arg_bpoint);

  TCLAP::MultiArg<std::string> arg_tpoint(
      "q",
      "t-point",
      "Specification of a point in the tracker coordinate system.",
      false,
      "x,y,z");
  cmd.add(arg_tpoint);

  TCLAP::ValueArg<std::string> arg_outputfile(
      "o",
      "output-file",
      "The image file to write the results to.",
      false,
      "output.tif",
      "file");
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
    std::cerr << "Error: " << e.error() << " for arg " << e.argId()
              << std::endl;
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

  std::shared_ptr<gmTrack::ProjectionTextureGenerator> generator =
      std::make_shared<gmTrack::ProjectionTextureGenerator>();
  generator->setController(controller);
  generator->setInlierThreshold(arg_pos_inlier.getValue());
  generator->setOrientationInlierThreshold(arg_ori_inlier.getValue());

  if (!arg_bpoint.getValue().empty()) {

    auto str_points = arg_bpoint.getValue();

    for (auto str_point : str_points) {
      std::replace(str_point.begin(), str_point.end(), ',', ' ');
      std::stringstream ss(str_point);
      Eigen::Vector2f p;
      if (ss >> p)
        generator->addBufferPosition(p);
      else
        std::cerr << "Warning: could not parse '" << str_point
                  << "' as a 3D point." << std::endl;
    }

  } else if (config && config->hasParam("point")) {

    std::vector<std::string> str_points;
    config->getAllParams("point", str_points);

    for (auto str_point : str_points) {
      std::stringstream ss(str_point);
      Eigen::Vector2f p;
      if (ss >> p)
        generator->addBufferPosition(p);
      else
        std::cerr << "Warning: could not parse '" << str_point
                  << "' as a 3D point." << std::endl;
    }

  } else {
    std::cerr
        << std::endl
        << "Error: No points available for registering coordinate systems. Add 'point' parameters to the configuration or use the 'point' command line argument."
        << std::endl
        << std::endl;
    return 4;
  }

  if (!arg_tpoint.getValue().empty()) {

    auto str_points = arg_tpoint.getValue();

    for (auto str_point : str_points) {
      std::replace(str_point.begin(), str_point.end(), ',', ' ');
      std::stringstream ss(str_point);
      Eigen::Vector3f p;
      if (ss >> p)
        generator->addTrackerPosition(p);
      else
        std::cerr << "Warning: could not parse '" << str_point
                  << "' as a 3D point." << std::endl;
    }

  } else if (config && config->hasParam("trackerPoint")) {

    std::vector<std::string> str_points;
    config->getAllParams("trackerPoint", str_points);

    for (auto str_point : str_points) {
      std::stringstream ss(str_point);
      Eigen::Vector3f p;
      if (ss >> p)
        generator->addTrackerPosition(p);
      else
        std::cerr << "Warning: could not parse '" << str_point
                  << "' as a 3D point." << std::endl;
    }
  }

  std::string output_file = arg_outputfile.getValue();

  typedef gmCore::TimeTools::clock clock;
  clock::time_point start_time = clock::now();

  while (true) {
    gmCore::Updateable::updateAll();
    clock::time_point now_time = clock::now();

  }

  return 0;
}
