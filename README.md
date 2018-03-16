# gramods

Gramods, short for Graphics Modules, is a collection of weakly inter dependent and useful APIs for Virtual Reality and graphics cluster rendering.

## Repository Structure

The repository divides the software into modules and apps where the modules provide functionality that the apps put together into executables. All modules listed in the modules/ folder will be compiled into separate libraries, and all apps in the apps/ folder will be compiled into executables.

The CMakeLists.txt file will automatically generate a list of modules based on the folders found in the modules/ folder. CMake will make sure that all targets are compiled and linked in the right order, as long as dependencies are specified correctly. If new modules or apps are added after the initialization of the cmake cache, by a git pull or manually, this must be manually added to its list.

## Exampel

```c++
int main(int argc, char *argv[]) {
  gmConfig::Configuration config(argc, argv);

  std::shared_ptr<gmNetSync::SyncNode> cluster_sync;
  if (! config.getObject(cluster_sync)) {
    ERROR("Cannot run without cluster synchronization settings!");
    return -1;
  }

  std::shared_ptr<gmGraphics::Pipeline> graphics;
  if (! config.getObject(graphics)) {
    ERROR("Cannot run without graphics pipeline!");
    return -1;
  }

  std::shared_ptr<gmTracking::Tracker> head_tracker;
  config.getObject("head tracker", head_tracker);

  std::shared_ptr<gmTracking::Tracker> wand_primary;
  config.getObject("primary controller", wand_primary);

  std::shared_ptr<gmTracking::Tracker> wand_secondary;
  config.getObject("secondary controller", wand_secondary);

  ...

  while (cluster_sync->isAlive()) {
    cluster_sync->synchronizeAllStates();
    graphics->renderFullPipeline(myRenderFunction);

    ...
  }
```

## Modules and Dependencies

### gmConfig

Utilities for loading other modules and configure them based on configuration files.

Requirements: TinyXML2

