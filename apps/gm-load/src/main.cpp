

#include <gmCore/Configuration.hh>
#include <gmCore/CommandLineParser.hh>
#include <gmCore/OStreamMessageSink.hh>
#include <gmCore/Updateable.hh>
#include <gmCore/ExitException.hh>
#include <gmCore/TimeTools.hh>

#include <gmGraphics/Window.hh>
#include <gmGraphics/CallbackRenderer.hh>

#include <gmNetwork/SyncNode.hh>
#include <gmNetwork/RunSync.hh>
#include <gmNetwork/DataSync.hh>
#include <gmNetwork/SyncSData.hh>

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

  TCLAP::SwitchArg output_time(
      "t", "show-time",
      "Log timing information every second second.",
      cmd, false);

  TCLAP::SwitchArg sync_start("","sync-start","Wait for the other nodes specified in the configuration, to start at the same time.", cmd, false);
  TCLAP::SwitchArg sync_swap("","sync-swap","Synchronize swap buffers with the other nodes specified in the configuration.", cmd, false);
  TCLAP::SwitchArg sync_time("","sync-time","Synchronize update time with the other nodes specified in the configuration. Observe that this will assume that peer 0 (zero) is the primary, activate synchronization and that DataSync::update() will be automatically called before the rendering pass.", cmd, false);

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

  bool is_primary_and_sync_time = false;
  gmNetwork::RunSync * run_sync = nullptr;
  gmNetwork::DataSync * data_sync = nullptr;
  std::shared_ptr<gmNetwork::SyncNode> sync_node;
  if (config->getObject(sync_node)) {
    run_sync = sync_node->getProtocol<gmNetwork::RunSync>();
    data_sync = sync_node->getProtocol<gmNetwork::DataSync>();
    is_primary_and_sync_time =
        sync_time.getValue() && sync_node->getLocalPeerIdx() == 0;
  }

  if (sync_start.getValue() && run_sync == nullptr)
    std::cerr << "Cannot synchronize start of execution"
                 " - no SyncNode found in the configuration.\n";
  if (sync_swap.getValue() && run_sync == nullptr)
    std::cerr << "Cannot synchronize swap buffers"
                 " - no SyncNode found in the configuration.\n";

  if (sync_time.getValue() && data_sync == nullptr)
    std::cerr << "Cannot synchronize time "
                 "- no SyncNode found in the configuration.\n";

  std::shared_ptr<gmNetwork::SyncSFloat64> shared_time =
      std::make_shared<gmNetwork::SyncSFloat64>(0.0);
  if (sync_time.getValue() && data_sync != nullptr)
    data_sync->addData(shared_time);

  std::vector<std::shared_ptr<gmCore::Object>> objects;
  config->getAllObjects(objects);

  typedef std::chrono::steady_clock clock;
  typedef std::chrono::duration<double, std::ratio<1>> d_seconds;

  auto last_print_time = clock::now();
  size_t frame_count = 0;
  size_t frame_number = 0;

  d_seconds update_time = d_seconds();
  d_seconds tsync_time = d_seconds();
  d_seconds render_time = d_seconds();
  d_seconds wait_time = d_seconds();
  d_seconds swap_time = d_seconds();
  d_seconds vsync_time = d_seconds();

  if ((sync_start.getValue() || sync_swap.getValue() || sync_time.getValue()) &&
      sync_node != nullptr) {
    sync_node->waitForConnection();
    run_sync->wait();
  }

  int exit_code = 0;

  try {
    bool alive = true;
    while (alive) {

      alive = windows.empty();

      if (is_primary_and_sync_time)
        *shared_time = gmCore::TimeTools::timePointToSeconds(clock::now());

      std::vector<clock::time_point> times;
      times.push_back(clock::now());

      for (auto window : windows) window->processEvents();

      // update_time
      times.push_back(clock::now());

      if (sync_time.getValue() && data_sync != nullptr) {
        run_sync->wait();
        data_sync->update();
        gmCore::Updateable::updateAll(
            gmCore::TimeTools::secondsToTimePoint(*shared_time), frame_number);
      } else {
        gmCore::Updateable::updateAll(clock::now(), frame_number);
      }

      // tsync_time
      times.push_back(clock::now());

      for (auto window : windows) {
        if (!window->isOpen()) continue;
        window->renderFullPipeline(frame_number);
      }

      // render_time
      times.push_back(clock::now());

      if (sync_swap.getValue() && run_sync != nullptr)
        run_sync->wait();

      // wait_time
      times.push_back(clock::now());

      for (auto window : windows) {
        if (!window->isOpen()) continue;
        window->swap();
        alive |= true;
      }

      // swap_time
      times.push_back(clock::now());

      for (auto window : windows) {
        if (!window->isOpen()) continue;
        window->sync();
      }

      // vsync_time
      times.push_back(clock::now());

      ++frame_number;
      ++frame_count;

      {
        auto times_it = times.begin();
        update_time += std::chrono::duration_cast<d_seconds>(*(times_it + 1) - *times_it);
        times_it++; assert(times_it != times.end());
        tsync_time += std::chrono::duration_cast<d_seconds>(*(times_it + 1) - *times_it);
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

      if (output_time.getValue() && dt.count() > 2) {

        const float to_us = 1e6f / (float)frame_count;

        GM_INF("gm-load", "Running at rate " << (frame_count / dt.count()) << " fps\n"
               << "updates:   " << (int)(to_us * update_time.count()) << " \u00B5s\n"
               << "rendering: " << (int)(to_us * render_time.count()) << " \u00B5s");
        if (sync_time.getValue()) {
          GM_INF("gm-load", "t-sync:    " << (int)(to_us * tsync_time.count()) << " \u00B5s");
        }
        if (sync_swap.getValue()) {
          GM_INF("gm-load",
                 "swapping:  " << (int)(to_us * (wait_time.count() +
                                                 swap_time.count())) << " \u00B5s"
                 << " (including " << (int)(to_us * wait_time.count()) << " \u00B5s sync)");
        } else {
          GM_INF("gm-load",
                 "swapping:  " << (int)(to_us * swap_time.count()) << " \u00B5s");
        }
        GM_INF("gm-load", "v-sync:    " << (int)(to_us * vsync_time.count()) << " \u00B5s");

        last_print_time = current_time;
        frame_count = 0;

        update_time = d_seconds();
        tsync_time = d_seconds();
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
#ifdef NDEBUG
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
#endif

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
#ifdef NDEBUG
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
#endif
  }

  return exit_code;
}
