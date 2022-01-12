

#include <gmCore/Configuration.hh>
#include <gmCore/Updateable.hh>
#include <gmCore/ExitException.hh>

#include <gmGraphics/Window.hh>
#include <gmGraphics/CallbackRenderer.hh>

#include <gmNetwork/SyncNode.hh>

#include <chrono>

#include "MyApp.hh"

using namespace gramods;

int main(int argc, char *argv[]) {

  // Read configuration
  std::unique_ptr<gmCore::Configuration> config;
  try {
    config = std::make_unique<gmCore::Configuration>(argc, argv);
  } catch (const gmCore::RuntimeException &e) {
    std::cerr << "Configuration error: " << e.what << std::endl;
  } catch (...) {
    std::cerr << "Unknown internal error while creating Configuration instance."
              << std::endl;
  }

  if (!config) {
    std::cerr << std::endl
              << "No valid configuration available." << std::endl
              << std::endl;
    return -1;
  }

  // Extract SyncNode to hand over to MyApp
  std::vector<std::shared_ptr<gmNetwork::SyncNode>> sync_nodes;
  config->getAllObjects(sync_nodes);

  // Extract Controller (Wand) to hand over to MyApp
  std::vector<std::shared_ptr<gmTrack::Controller>> controllers;
  config->getAllObjects(controllers);

  // Extract head tracker to hand over to MyApp
  std::shared_ptr<gmTrack::SinglePoseTracker> head;
  config->getObjectByDef("head", head);

  // Instantiate MyApp and hand over SyncNode and Controller. We do
  // not care if the lists are empty or not, that is for MyApp to take
  // into consideration.
  std::shared_ptr<MyApp> myapp =
    std::make_shared<MyApp>(sync_nodes, controllers, head);

  // Extract Window, because we need to call them to process events,
  // render and swap buffers.
  std::vector<std::shared_ptr<gmGraphics::Window>> windows;
  config->getAllObjects(windows);
  if (!windows.empty())
    // Add our app to the first window since we do not currently
    // support multiple OpenGL contexts
    windows[0]->addRenderer(myapp->getRenderer());

  // Extract all other objects, just in case there are things in the
  // config that needs to be loaded
  std::vector<std::shared_ptr<gmCore::Object>> objects;
  config->getAllObjects(objects);

  typedef std::chrono::steady_clock clock;
  typedef std::chrono::duration<double, std::ratio<1>> d_seconds;

  auto last_print_time = clock::now();
  size_t frame_count = 0;
  size_t frame_number = 0;

  d_seconds update_time = d_seconds();
  d_seconds render_time = d_seconds();
  d_seconds swap_time = d_seconds();
  d_seconds vsync_time = d_seconds();

  int exit_code = 0;

  try {
    bool alive = true;
    while (alive) {

      alive = windows.empty();

      auto t0 = clock::now();

      for (auto window : windows) { window->processEvents(); }
      gmCore::Updateable::updateAll();

      auto t1 = clock::now();

      for (auto window : windows) {
        if (!window->isOpen()) continue;
        window->renderFullPipeline(frame_number);
      }

      auto t2 = clock::now();

      for (auto window : windows) {
        if (!window->isOpen()) continue;
        window->swap();
        alive |= true;
      }

      auto t3 = clock::now();

      for (auto window : windows) {
        if (!window->isOpen()) continue;
        window->sync();
      }

      auto t4 = clock::now();

      ++frame_number;
      ++frame_count;

      update_time += std::chrono::duration_cast<d_seconds>(t1 - t0);
      render_time += std::chrono::duration_cast<d_seconds>(t2 - t1);
      swap_time += std::chrono::duration_cast<d_seconds>(t3 - t2);
      vsync_time += std::chrono::duration_cast<d_seconds>(t4 - t3);

      auto current_time = clock::now();
      auto dt =
          std::chrono::duration_cast<d_seconds>(current_time - last_print_time);
      if (dt.count() > 2) {
        float to_us = 1e6f / (float)frame_count;
        GM_INF("gm-demo-osg",
               "Running at rate "
                   << (frame_count / dt.count()) << " fps" << std::endl
                   << "updates:   " << (int)(to_us * update_time.count())
                   << " \u00B5s" << std::endl
                   << "rendering: " << (int)(to_us * render_time.count())
                   << " \u00B5s" << std::endl
                   << "swapping:  " << (int)(to_us * swap_time.count())
                   << " \u00B5s" << std::endl
                   << "v-sync:    " << (int)(to_us * vsync_time.count())
                   << " \u00B5s");

        last_print_time = current_time;
        frame_count = 0;

        update_time = d_seconds();
        render_time = d_seconds();
        swap_time = d_seconds();
        vsync_time = d_seconds();
      }
    }
  } catch (const gmCore::ExitException &e) {
    exit_code = e.exit_code;
  } catch (const gmCore::RuntimeException &e) {
    GM_ERR("gm-demo-org", "Terminated by runtime exception: " << e.what);
    exit_code = -2;
  } catch (...) {
    GM_ERR("gm-demo-org", "Terminated by unknown exception");
    exit_code = -255;
  }

  try {
    objects.clear();
    windows.clear();
    config.reset();
  } catch (const gmCore::ExitException &e) {
    GM_WRN("gm-demo-osg",
           "ExitException while already terminating with exit code "
               << exit_code << ".");
    exit_code = e.exit_code;
  } catch (const gmCore::RuntimeException &e) {
    GM_WRN("gm-demo-osg",
           "RuntimeException (" << e.what
                                << ") while already terminating with exit code "
                                << exit_code << ".");
    exit_code = -2;
  } catch (...) {
    GM_WRN("gm-demo-osg",
           "Unknown exception while already terminating with exit code "
               << exit_code << ".");
    exit_code = -255;
  }

  return exit_code;
}
