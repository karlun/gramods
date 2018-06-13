

#include <gmCore/Configuration.hh>
#include <gmCore/CommandLineParser.hh>
#include <gmCore/OStreamMessageSink.hh>
#include <gmTrack/Controller.hh>

#include <tclap/CmdLine.h>

#include <BaseEstimator.hh>

#include <fstream>

using namespace gramods;

int main(int argc, char *argv[]) {

  TCLAP::CmdLine cmd
    ("Tool for semi-automatically registering a tracker coordinates system into a room coordinates system. This is done by, in turn, marking the origin and the +x and +y directions, alternatively three or four pre-defined points in 3D space.");

  TCLAP::ValueArg<std::string> arg_coords_file
    ("", "coords",
     "Output filename for the estimated tracker coordinates registration.",
     true, "", "file", cmd);

  TCLAP::MultiArg<std::string> arg_config_dummy
    ("", "config",
     "Configuration file to read controller definition from.",
     true, "file");
  TCLAP::MultiArg<std::string> arg_xml_dummy
    ("", "xml",
     "XML configuration to read controller definition from.",
     true, "string");
  cmd.xorAdd(arg_config_dummy, arg_xml_dummy);

  TCLAP::ValueArg<std::string> arg_controller_name
    ("", "controller-name",
     "Name of the controller to use, in case multiple controllers are defined in the configuration file.",
     false, "", "string", cmd);

  TCLAP::SwitchArg arg_scale
    ("f", "free-scale",
     "Activates free scaling in the registration. Typically uniform scaling is what you want.",
     false);
  TCLAP::SwitchArg arg_scale_uniform
    ("u", "uniform-scale",
     "Activates uniform scaling, for example changing coordinates unit together with the registration.",
     false);
  cmd.add(arg_scale_uniform);
  cmd.add(arg_scale);

  try {
    cmd.parse(argc, argv);
  } catch (TCLAP::ArgException &e) {
    std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
    return -1;
  }

  if (arg_scale.getValue() && arg_scale_uniform.getValue()) {
    TCLAP::CmdLineParseException e("Arguments free-scale and uniform-scale both set!");
    cmd.getOutput()->failure(cmd, e);
    return -1;
  }

  std::stringstream internal_messages_ss;
  std::shared_ptr<gmCore::OStreamMessageSink> osms =
    std::make_shared<gmCore::OStreamMessageSink>();
  osms->setStream(&internal_messages_ss);
  osms->initialize();

  std::unique_ptr<gmCore::Configuration> config;
  try {
    config = std::make_unique<gmCore::Configuration>(argc, argv);
  }
  catch (const std::exception &ex) {
    std::cerr << "Configuration error: " << ex.what() << std::endl;
  }
  catch(...) {
    std::cerr << "Unknown internal error while creating Configuration instance." << std::endl;
  }

  if (!config) {
    std::cerr << std::endl
              << "No valid configuration available." << std::endl
              << std::endl
              << "Internal messages:" << std::endl
              << internal_messages_ss.str() << std::endl
              << std::endl;
    return -1;
  }

  std::shared_ptr<gmTrack::Controller> controller;

  if (arg_controller_name.isSet()) {
    if (!config->getObject(arg_controller_name.getValue(), controller)) {
      std::cerr << std::endl
                << "Could not find controller named "
                << "'" << arg_controller_name.getValue() << "'"
                << " in the specified configuration." << std::endl
                << std::endl
                << "Internal messages:" << std::endl
                << internal_messages_ss.str() << std::endl
                << std::endl;
      return -1;
    }
  } else {
    if (!config->getObject(controller)) {
      std::cerr << std::endl
                << "Could not find controller in the specified configuration." << std::endl
                << std::endl
                << "Internal messages:" << std::endl
                << internal_messages_ss.str() << std::endl
                << std::endl;
      return -1;
    }
  }

  {
    std::ofstream fout(arg_coords_file.getValue());
    if (!fout) {
      std::cerr << "Could not open output file '" << arg_coords_file.getValue() << "'" << std::endl;
      return -1;
    }
  }

  BaseEstimator be;
  be.setScale(arg_scale.getValue());
  be.setUniform(arg_scale_uniform.getValue());
  be.setController(controller);

  int ret = be.process();
  if (ret)
    return ret;

  auto T = be.getBase();

  std::ofstream fout(arg_coords_file.getValue());
  fout << T;

  return 0;
}
