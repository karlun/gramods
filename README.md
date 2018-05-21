# gramods

Gramods, short for Graphics Modules, is a collection of weakly inter dependent and useful APIs for Virtual Reality and graphics cluster rendering.

## Repository Structure

The repository divides the software into modules and apps where the modules provide functionality that the apps put together into executables. All modules listed in the modules/ folder will be compiled into separate libraries, and all apps in the apps/ folder will be compiled into executables.

The CMakeLists.txt file will automatically generate a list of modules based on the folders found in the modules/ folder. CMake will make sure that all targets are compiled and linked in the right order, as long as dependencies are specified correctly. If new modules or apps are added after the initialization of the cmake cache, by a git pull or manually, this must be manually added to its list.

## Purpose

The purpose of the Gramods library is to simplify loading of platform dependent configurations into a pre-compiled application, while also simplifying the implementation of such applications. The aim is to be able to implement an application in a way similar to this example code:

```c++
int main(int argc, char *argv[]) {

  gmCore::Configuration config(argc, argv);

  std::shared_ptr<gmNetwork::NetSync> cluster_sync;
  if (! config.getObject(cluster_sync)) {
    ERROR("Cannot run without cluster synchronization settings!");
    return -1;
  }

  std::shared_ptr<gmGraphics::Pipeline> graphics;
  if (! config.getObject(graphics)) {
    ERROR("Cannot run without graphics pipeline!");
    return -1;
  }

  GM_INF("main", "Configuration loaded");

  std::shared_ptr<gmTracking::Tracker> head_tracker;
  config.getObject("head tracker", head_tracker);
  GM_INF("main", "Head tracker: " << head_tracker);

  std::shared_ptr<gmTracking::Tracker> wand_primary;
  config.getObject("primary controller", wand_primary);
  GM_INF("main", "Primary controller: " << head_tracker);

  ...

  graphics->callOnceWithGLContext([=] (gmGraphics::FrameData info) {
      myInitFunction(info.getGLContextIndex());
    });

  while (cluster_sync->isAlive()) {

    if (cluster_sync->isMaster())
      update_master_states();
    cluster_sync->synchronizeAllStates();
    update_dependent_states();

    graphics->renderFullPipeline([=] (gmGraphics::FrameData info) {
        myRenderFunction(info.getFrustum(), info.getViewMatrix());
      });

    cluster_sync->waitForAll();
    graphics->swap();
    ...
  }
```

# Modules and Dependencies

## gmCore

Utilities for loading other modules and configure them based on configuration files, and for handling library and application debugging output.

Requirements:
 - TinyXML2

### Module Program Design Principles

Complex object factory instantiation, XML and command line parameter control, and both internal and application level debugging information are encapsulated and hidden behind purpose specific interfaces and easy-to-use macros. At configuration time, from XML or command line arguments, there is an association between a attribute string and a corresponding class method setting this parameter. After instantiation, however, client code must instead call the methods.

A Simple example of how configuration works:

```c++
/// Typically in header (myclass.hh)
struct MyClass : gramods::gmCore::Object {
  int parameter;
  void setParameter(int value) { parameter = value; }
  GM_OFI_DECLARE(MyClass);
};

/// Typically in c++ file (myclass.cpp)
GM_OFI_DEFINE(MyClass);
GM_OFI_PARAM(MyClass, parameter, int, MyClass::setParameter);
```

This class can then be instantiated by loading this configuration file:

```xml
<config>
  <MyClass parameter="5"/>
</config>
```

and the `parameter` value can be overridden by command line `--param MyClass.parameter=3`. This is when reading the configuration file using command line arguments, like this:

```c++
int main(int argc, char *argv[]) {
  gmCore::Configuration config(argc, argv);

  std::shared_ptr<MyClass> node;
  config.getObject(node);

  std::cout << node.parameter << std::endl;
}
```

## gmTrack

Pose tracking client classes and filters.

Required dependences:
 - Eigen

Optional dependences:
 - VRPN, for VRPN support

### Module Program Design Principles

Abstraction of tracking using Decorator design pattern for flexible filtering, calibration and registration with minimal code duplication.

## gmNetwork (to be defined)

Network data and execution synchronization.

Required dependences:
 - ASIO (with or without Boost)

### Module Program Design Principles

Thread encapsulation and role agnostic synchronization.

## gmGraphics (to be defined)

Graphics rendering pipeline definition and handling.

Required dependences:
 - GLObjects

### Module Program Design Principles

A core principles of the graphics module is that application design parts are encapsulated in descriptive classes such as window, for producing output into a window, and tile, to specify tiles within windows. There is also an generalized abstraction of processing steps so that it should be easy to rearrange existing steps into new effects and to inject new post processing steps.

```plantuml
client - Pipeline
Pipeline -> Window
Window -> Tile
Tile --> Camera
note "does post processing through OpenGL" as N1
Tile -> Processor
Processor -- N1
Processor --> Camera
```
