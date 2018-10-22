

#include <gmCore/Configuration.hh>
#include <gmCore/CommandLineParser.hh>
#include <gmCore/OStreamMessageSink.hh>
#include <gmCore/Updateable.hh>

#include <gmGraphics/Window.hh>
#include <gmGraphics/CallbackRenderer.hh>

#include <tclap/CmdLine.h>

#include <chrono>


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

  typedef std::chrono::steady_clock clock;
  typedef std::chrono::duration<double, std::ratio<1>> d_seconds;

  auto last_print_time = std::chrono::steady_clock::now();
  size_t frame_count = 0;

  d_seconds update_time;
  d_seconds render_time;
  d_seconds swap_time;
  d_seconds vsync_time;

  bool alive = true;
  while (alive) {

    alive = windows.empty();

    auto t0 = std::chrono::steady_clock::now();

    gmCore::Updateable::updateAll();
    for (auto window : windows) {
      window->processEvents();
    }

    auto t1 = std::chrono::steady_clock::now();

    for (auto window : windows) {
      if (!window->isOpen()) continue;
      window->RendererDispatcher::renderFullPipeline();
    }

    auto t2 = std::chrono::steady_clock::now();

    for (auto window : windows) {
      if (!window->isOpen()) continue;
      window->swap();
      alive |= true;
    }

    auto t3 = std::chrono::steady_clock::now();

    for (auto window : windows) {
      if (!window->isOpen()) continue;
      window->sync();
    }

    auto t4 = std::chrono::steady_clock::now();

    frame_count += 1;

    update_time += std::chrono::duration_cast<d_seconds>(t1 - t0);
    render_time += std::chrono::duration_cast<d_seconds>(t2 - t1);
    swap_time += std::chrono::duration_cast<d_seconds>(t3 - t2);
    vsync_time += std::chrono::duration_cast<d_seconds>(t4 - t3);

    auto current_time = std::chrono::steady_clock::now();
    auto dt = std::chrono::duration_cast<d_seconds>(current_time - last_print_time);
    if (dt.count() > 2) {
      float to_us = 1e6 / (float)frame_count;
      GM_INF("gm-load", "Running at rate " << (frame_count / dt.count()) << " fps" << std::endl
             << "updates:   " << (int)(to_us * update_time.count()) << " \u00B5s" << std::endl
             << "rendering: " << (int)(to_us * render_time.count()) << " \u00B5s" << std::endl
             << "swapping:  " << (int)(to_us * swap_time.count()) << " \u00B5s" << std::endl
             << "v-sync:    " << (int)(to_us * vsync_time.count()) << " \u00B5s");

      last_print_time = current_time;
      frame_count = 0;

      update_time = d_seconds();
      render_time = d_seconds();
      swap_time = d_seconds();
      vsync_time = d_seconds();
    }
  }

  return 0;
}
