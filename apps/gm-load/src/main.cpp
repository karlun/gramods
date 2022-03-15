

#include <gmCore/Configuration.hh>
#include <gmCore/CommandLineParser.hh>
#include <gmCore/OStreamMessageSink.hh>
#include <gmCore/Updateable.hh>
#include <gmCore/ExitException.hh>

#include <gmGraphics/Window.hh>
#include <gmGraphics/CallbackRenderer.hh>

#include <gmNetwork/SyncNode.hh>
#include <gmNetwork/RunSync.hh>

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

	TCLAP::SwitchArg sync_start("s","sync-start","Wait for the other nodes specified in the configuration, to start at the same time.", cmd, false);
	TCLAP::SwitchArg sync_swap("w","sync-swap","Synchronize swap buffers with the other nodes specified in the configuration.", cmd, false);

  try {
    cmd.parse(argc, argv);
  } catch (const TCLAP::ArgException &e) {
    std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
    return -1;
  }

  std::unique_ptr<gmCore::Configuration> config;
  try {
    config = std::make_unique<gmCore::Configuration>(argc, argv);
  }
  catch (const gmCore::RuntimeException &e) {
    std::cerr << "Runtime exception while creating Configuration instance: "
              << e.what << std::endl;
  }
  catch (const std::exception &e) {
    std::cerr << "Unknown exception while creating Configuration instance: "
              << e.what() << std::endl;
  }
  catch (...) {
    std::cerr << "Unknown exception while creating Configuration instance."
              << std::endl;
  }

  if (!config) {
    std::cerr << std::endl
              << "No valid configuration available." << std::endl
              << std::endl;
    return -1;
  }

  std::vector<std::shared_ptr<gmGraphics::Window>> windows;
  config->getAllObjects(windows);

  gmNetwork::RunSync * run_sync = nullptr;
  std::shared_ptr<gmNetwork::SyncNode> sync_node;
  if (config->getObject(sync_node))
    run_sync = sync_node->getProtocol<gmNetwork::RunSync>();

  if (sync_start.getValue() && run_sync == nullptr)
    std::cerr << "Cannot synchronize start of execution"
      " - no RunSync found in the configuration." << std::endl;
  if (sync_swap.getValue() && run_sync == nullptr)
    std::cerr << "Cannot synchronize swap buffers"
      " - no RunSync found in the configuration." << std::endl;

  std::vector<std::shared_ptr<gmCore::Object>> objects;
  config->getAllObjects(objects);

  typedef std::chrono::steady_clock clock;
  typedef std::chrono::duration<double, std::ratio<1>> d_seconds;

  auto last_print_time = clock::now();
  size_t frame_count = 0;
  size_t frame_number = 0;

  d_seconds update_time = d_seconds();
  d_seconds render_time = d_seconds();
  d_seconds wait_time = d_seconds();
  d_seconds swap_time = d_seconds();
  d_seconds vsync_time = d_seconds();

  if (sync_start.getValue() || sync_swap.getValue())
    sync_node->waitForConnection();

  int exit_code = 0;

  try {
    bool alive = true;
    while (alive) {

      alive = windows.empty();

      std::vector<clock::time_point> times;
      times.push_back(clock::now());

      for (auto window : windows) {
        window->processEvents();
      }
      gmCore::Updateable::updateAll();

      times.push_back(clock::now());

      for (auto window : windows) {
        if (!window->isOpen()) continue;
        window->renderFullPipeline(frame_number);
      }

      times.push_back(clock::now());

      if (sync_swap.getValue())
        run_sync->wait();

      times.push_back(clock::now());

      for (auto window : windows) {
        if (!window->isOpen()) continue;
        window->swap();
        alive |= true;
      }

      times.push_back(clock::now());

      for (auto window : windows) {
        if (!window->isOpen()) continue;
        window->sync();
      }

      times.push_back(clock::now());

      ++frame_number;
      ++frame_count;

      {
        auto times_it = times.begin();
        update_time += std::chrono::duration_cast<d_seconds>(*(times_it + 1) - *times_it);
        times_it++; assert(times_it != times.end());
        render_time += std::chrono::duration_cast<d_seconds>(*(times_it + 1) - *times_it);
        times_it++; assert(times_it != times.end());
        wait_time += std::chrono::duration_cast<d_seconds>(*(times_it + 1) - *times_it);
        times_it++; assert(times_it != times.end());
        swap_time += std::chrono::duration_cast<d_seconds>(*(times_it + 1) - *times_it);
        times_it++; assert(times_it != times.end());
        vsync_time += std::chrono::duration_cast<d_seconds>(*(times_it + 1) - *times_it);
      }

      auto current_time = clock::now();
      auto dt = std::chrono::duration_cast<d_seconds>(current_time - last_print_time);

      if (dt.count() > 2) {

        float to_us = 1e6f / (float)frame_count;

        if (sync_swap.getValue()) {
          GM_INF("gm-load", "Running at rate " << (frame_count / dt.count()) << " fps" << std::endl
                 << "updates:   " << (int)(to_us * update_time.count()) << " \u00B5s" << std::endl
                 << "rendering: " << (int)(to_us * render_time.count()) << " \u00B5s" << std::endl
                 << "swapping:  " << (int)(to_us * (wait_time.count() +
                                                    swap_time.count())) << " \u00B5s"
                 << " (including " << (int)(to_us * wait_time.count()) << " \u00B5s synchronization)" << std::endl
                 << "v-sync:    " << (int)(to_us * vsync_time.count()) << " \u00B5s");
        } else {
          GM_INF("gm-load", "Running at rate " << (frame_count / dt.count()) << " fps" << std::endl
                 << "updates:   " << (int)(to_us * update_time.count()) << " \u00B5s" << std::endl
                 << "rendering: " << (int)(to_us * render_time.count()) << " \u00B5s" << std::endl
                 << "swapping:  " << (int)(to_us * wait_time.count()) << " \u00B5s" << std::endl
                 << "v-sync:    " << (int)(to_us * vsync_time.count()) << " \u00B5s");
        }

        last_print_time = current_time;
        frame_count = 0;

        update_time = d_seconds();
        render_time = d_seconds();
        wait_time = d_seconds();
        swap_time = d_seconds();
        vsync_time = d_seconds();
      }
    }
  }
  catch (const gmCore::ExitException &e) {
    exit_code = e.exit_code;
  }
  catch (const gmCore::RuntimeException &e) {
    GM_ERR("gm-load", "Terminated by runtime exception: " << e.what);
    exit_code = -2;
  }
  catch (const std::exception &e) {
    GM_ERR("gm-load", "Terminated by unknown exception: " << e.what());
    exit_code = -3;
  }
  catch (...) {
    GM_ERR("gm-load", "Terminated by unknown exception.");
    exit_code = -255;
  }


  try {
    objects.clear();
    sync_node.reset();
    windows.clear();
    config.reset();
  } catch (const gmCore::ExitException &e) {
    GM_WRN("gm-load",
           "ExitException (code "
               << e.exit_code << ") while already terminating with exit code "
               << exit_code << ".");
  } catch (const gmCore::RuntimeException &e) {
    GM_WRN("gm-load",
           "RuntimeException (" << e.what
                                << ") while already terminating with exit code "
                                << exit_code << ".");
  } catch (const std::exception &e) {
    GM_WRN("gm-load",
           "Unknown exception ("
               << e.what() << ") while already terminating with exit code "
               << exit_code << ".");
  } catch (...) {
    GM_WRN("gm-load",
           "Unknown exception while already terminating with exit code "
               << exit_code << ".");
  }

  return exit_code;
}
