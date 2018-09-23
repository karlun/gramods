

#include <gmCore/Configuration.hh>
#include <gmCore/CommandLineParser.hh>
#include <gmCore/OStreamMessageSink.hh>

#include <gmGraphics/Window.hh>
#include <gmGraphics/CallbackRenderer.hh>

#include <tclap/CmdLine.h>


using namespace gramods;

int main(int argc, char *argv[]) {

  TCLAP::CmdLine cmd
    ("This is a simple configuration loader for gramods. Any functionality is encoded directly in the loaded configurations.");

  TCLAP::MultiArg<std::string> arg_config_dummy
    ("", "config",
     "Configuration file(s) load.",
     true, "file");
  TCLAP::MultiArg<std::string> arg_xml_dummy
    ("", "xml",
     "XML configuration(s) to load.",
     true, "string");
  cmd.xorAdd(arg_config_dummy, arg_xml_dummy);

  TCLAP::MultiArg<std::string> arg_param_dummy
    ("", "param",
     "Overrides configuration parameters. For example, if there is a node 'head' under the root and this node has a parameter 'connectionString' then this parameter can be overridden by '--param head.connectionString=WAND@localhost'.",
     false, "identifier=value");
  cmd.add(arg_param_dummy);

  try {
    cmd.parse(argc, argv);
  } catch (TCLAP::ArgException &e) {
    std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
    return -1;
  }

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
              << std::endl;
    return -1;
  }

  std::vector<std::shared_ptr<gmGraphics::Window>> windows;
  config->getAllObjects(windows);

  std::vector<std::shared_ptr<gmCore::Object>> objects;
  config->getAllObjects(objects);

  bool alive = true;
  while (alive) {

    alive = windows.empty();

    for (auto window : windows) {
      window->processEvents();
      window->renderFullPipeline();
      window->swap();

      alive |= window->isOpen();
    }
  }

  return 0;
}
