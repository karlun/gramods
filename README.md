# gramods

# Table of Content

1. Introduction
     1. Purpose
     2. Repository Structure
2. Build and Install
     1. Dependencies
     2. Standard Build Procedure
     3. Simple Out-of-the-box Usage
3. Apps and Dependencies
     1. gm-load
     2. gm-tracker-registration
4. Modules and Dependencies
     1. gmCore
     2. gmTrack
     3. gmNetwork
     4. gmGraphics
     5. gmTouch
     6. gmMisc


# Introduction

Gramods, short for Graphics Modules, is a collection of weakly inter dependent and useful APIs for Virtual Reality and graphics cluster rendering.


## Purpose

The main purpose of the Gramods library is to simplify loading of platform dependent configurations into a pre-compiled application, while also simplifying the implementation of such applications. The aim is to be able to implement an application that employs execution and data synchronization over network and synchronized multi window rendering in a flexible and configurable way.


## Repository Structure

The repository divides the software into *modules* and *apps* where the modules provide functionality that the apps put together into executables. All modules listed in the modules/ folder will be compiled into separate libraries, and all apps in the apps/ folder will be compiled into executables.

The top level CMakeLists.txt file will automatically make recursive inclusion of the CMakeLists.txt files of the modules and apps. CMake will then make sure that all targets are compiled and linked in the right order, as long as dependencies are specified correctly.


# Build and Install

The Gramods modules are designed to be weakly dependent on each other and on third party dependencies, even though they makes heavy use of third party dependencies for their functionality. Therefore, some modules will not be built without their core dependencies, while others will be built with limited functionality.


## Dependencies

Use CMake to find dependencies and to set up the build environment. Every modules, app and dependency is activated when found, however dependencies can be individually deactivated through CMake by setting *gramods_ENABLE_* to false in CMake, and modules and apps by setting *gramods_MODULE_* and *gramods_APP_*, respectively, to false.

Observe that CMake cannot automatically deactivate dependent modules when a modules is made unavailable, by a missing dependency or by deactivation, so this will result in a build error.

Most dependencies can be automatically installed and handled, with *vcpkg* through `vcpkg install asio eigen3 freeimage glew sdl2 tclap tinyobjloader tinyxml2`, or with *apt* through `apt install libasio-dev libeigen3-dev libfreeimage-dev libglew-dev libsdl2-dev libtclap-dev libtinyobjloader-dev libtinyxml2-dev`.

If CMake config files are missing in your installation but needed by Gramods, then the `*_DIR` can be pointed to a backup CMake config folder in the `Gramods/cmake_modules` folder. This will expose variables in CMake to manually point at include and lib folders for the library.


## Standard Build Procedure

After installing the necessary dependencies, a typical build and install would look something like this:

~~~~~~~~~~~~~{.sh}
cmake                                                                   ^
  -S gramods/ -B gramods/build/                                         ^
  -DCMAKE_TOOLCHAIN_FILE=path-to-vcpkg/scripts/buildsystems/vcpkg.cmake ^
  -DCMAKE_INSTALL_PREFIX=gramods/install
cmake --build gramods/build --target install --config Release
~~~~~~~~~~~~~

Depending on your local platform and its configuration you might want to also set `VCPKG_APPLOCAL_DEPS` to `OFF` and/or `gramods_INSTALL_DEPENDENCIES` to `ON`. The first flag controls copying dependencies into the build folder and `gramods_INSTALL_DEPENDENCIES` controls copying dependencies into the install tree.


## Simple Out-of-the-box Usage

Gramods is primarily a library for implementing your own applications, however there are also some apps included in the package. The simplest to use for testing Gramods functionality is `gm-load`. When the package has been fully built and installed, use the following command-line to run test configurations:

~~~~~~~~~~~~~{.sh}
gm-load --config urn:gramods:config/output-normal-to-console.xml --config urn:gramods:gmGraphics/config/test-TileViewOfCubeSet.xml
~~~~~~~~~~~~~


# Apps and Dependencies

The Gramods package includes a number of apps, for the purpose of testing or demonstrating Gramods functionality, or for running Gramods configurations.


## gm-load

The `gm-load` app loads one or more configuration files and executes them by calling `Updateable::updateAll` and by calling `Window` instances if such are defined in the configuration.

This app will not be built if these required dependencies are not configured for:

 - gmCore
 - gmGraphics
 - TinyXML2
 - TCLAP


## gm-tracker-registration

The `gm-tracker-registration` app does not open any graphical interface, but instead silently (depending on output verbosity) registers tracker positions, and subsequently calculates and outputs tracker registration data. It uses the top-most `Controller` in the specified configuration for reading off tracker position data and registers these against real world position data specified on the command line.

Whenever the main button (`ButtonsMapper::MAIN`) is pressed, the app samples data. If more than one data sample was captured during the button press, an average is calculated (IQM) and used. The app then estimates sphericity of the data and if three dimensions are considere linearly independent a full registration is automatically either solved or estimated, using least-squares estimation. If the data only span two dimensions the registered points are automatically expanded into the third dimension, assuming uniformity of both the tracker and real world coordinates, before estimating the registration.

The estimated registration data are written to a specified output file, `output.xml` per default, based on an output template. This template can be in any text-based format and the following keys are replaced by their corresponding registration data:

| Keys              | Data                                                                 |
|-------------------|----------------------------------------------------------------------|
| `%M0x %M0y %M0z`  | The first vector of the rotation part of the registration 4x4 matrix |
| `%M1x %M1y %M1z`  | The first vector of the rotation part of the registration 4x4 matrix |
| `%M2x %M2y %M2z`  | The first vector of the rotation part of the registration 4x4 matrix |
| `%M3x %M3y %M3z`  | The translation vector part of the registration 4x4 matrix           |
| `%Rx %Ry %Rz`     | The rotation axis of the rotation part of the registration           |
| `%Ra %Rd`         | The rotation angle expressed in radians and degrees, respectively    |
| `%Qx %Qy %Qz %Qw` | The rotation part expressed in quaternion format                     |

`%M`, `%R` and `%Q` can also be replaced with `%Mi`, `%Ri` and `%Qi,` respectively, for use of the inverse transform instead of the forward registration transform. To enter a literal `%` use `%%`.

This app will not be built if these required dependencies are not configured for:

 - gmCore
 - gmTrack
 - TCLAP


# Modules and Dependencies

The Gramods package divides the functionality into modules that can be built individually, given that the necessary dependencies are met. Some modules do have inter dependencies, however.

@startuml component
component gmCore
component gmNetwork
component gmTrack
component gmGraphics
component gmMisc
component gmTouch
gmCore <-- gmNetwork
gmCore <- gmGraphics
gmCore <-- gmTrack
gmCore <-- gmTouch
gmMisc <-- gmTouch
gmTrack <-- gmGraphics
@enduml


## gmCore

The gmCore module specifies types and utilities for loading other modules and configure them based on configuration files, for handling library and application error, warning and debugging output, and possibly also for initialization of third party libraries that may be used by other modules.

The module also provides types and operators for use in the other modules. In particular, it defines stream operator for advanced parsing of types such as Eigen::Quaternionf for orientation, and float and size_t array types.

Optional dependencies:

 - Eigen3 (at least version 3.3), for types and operators for vectors, quaternion and matrix types
 - TinyXML2, for XML-based configuration
 - SDL2, for initialization of the SDL2 library


### Module Program Design Principles

Complex object factory instantiation, XML and command line parameter control, and both internal and application level debugging information are encapsulated and hidden behind purpose specific interfaces and easy-to-use macros. At configuration time, from XML or command line arguments, there is an association between a attribute string and a corresponding class method setting this parameter. After instantiation, however, client code must instead call the methods.

A Simple example of how configuration works:

~~~~~~~~~~~~~{.cpp}
/// Typically in header (myclass.hh)
struct MyClass : gramods::gmCore::Object {
  int parameter;
  void setParameter(int value) { parameter = value; }
  GM_OFI_DECLARE;
};

/// Typically in c++ file (myclass.cpp)
GM_OFI_DEFINE(MyClass);
GM_OFI_PARAM(MyClass, parameter, int, MyClass::setParameter);
~~~~~~~~~~~~~
This class can then be instantiated by loading this configuration file:

~~~~~~~~~~~~~{.xml}
<config>
  <MyClass parameter="5"/>
</config>
~~~~~~~~~~~~~

and the `parameter` value can be overridden by command line `--param MyClass.parameter=3`. This is when reading the configuration file using command line arguments, like this:

~~~~~~~~~~~~~{.cpp}
int main(int argc, char *argv[]) {
  gramods::gmCore::Configuration config(argc, argv);

  std::shared_ptr<MyClass> node;
  config.getObject(node);

  std::cout << node.parameter << std::endl;
}
~~~~~~~~~~~~~


## gmTrack

The gmTrack module provides primarily pose tracking clients, servers and filters.

Required dependencies:

 - gmCore

 - Eigen3

Optional dependencies:

 - VRPN, for VRPN support
 - OpenCV (version 4) with aruco module, for marker-based tracking


### Module Program Design Principles

Abstraction of tracking using Decorator design pattern for flexible filtering, calibration and registration with minimal code duplication.


## gmNetwork

The gmNetwork module provides network data synchronization and execution synchronization.

Required dependencies:

 - gmCore

 - ASIO (at least version 1.12)


### Module Program Design Principles

Thread encapsulation and role agnostic synchronization.


## gmGraphics

The gmGraphics module provides nodes primarily for graphics rendering pipeline definition and handling.

Required dependencies:

 - gmCore
 - gmTrack

 - Eigen3 (at least version 3.3)
 - GLEW
 - OpenGL

Optional dependencies:

 - SDL2, for SDL-based window
 - FreeImage, for reading and writing images
 - Gimp (binaries), for generating textures using Python-fu
 - Inkscape (binaries), for generating textures from SVG
 - libuvc, for UVC support, reading image data from video class USB devices.
 - OpenVR, for HMD and OpenVR tracking support
 - TinyObjLoader, for loading Wavefront files


### Module Program Design Principles

A Window creates a graphics context and makes it current before any subsequent calls. The Window calls a View to produce graphics. A View may call other views recursively, or make one or more calls to one or more renderers to produce this graphics. It is the renderer that actually renders the scene, while the view determines frame buffers and frustum settings to use, based on the current Viewpoint.

Support for stereoscopic rendering is provided through MultiscopicMultiplexer. If a MultiscopicView has a MultiscopicMultiplexer associated with it, it will call this to set up rendering to the left and right eye, or more if the view supports holographic rendering. Thus, the MultiscopicMultiplexer does not know how the eyes are physically oriented and offsetted, which even may be different depending on the View or Viewpoint, but only knows if the graphics should be rendered to quad buffers or interlaced or anaglyphic to the back buffer. A smart interlaced technique can therefore combine the masked out lines with the closest shown lines.

The TiledView node makes use of the *Decorator Design Pattern*, to allow for flexible configurations of graphics output.

@startuml class
RendererDispatcher <|-- Window
RendererDispatcher : renderFullPipeline(ViewSettings)
RendererDispatcher <|-- View
Window - View
View <|-- TiledView
View <|-- MultiscopicView
MultiscopicView <|-- SpatialPlanarView
MultiscopicView <|-- SpatialDomeView
View <|-- AngularFisheyeView
View <|-- EquirectangularView
View - Viewpoint
note top of Viewpoint : a View may not be affected by\nViewpoint orientation
Viewpoint : position
Viewpoint : orientation
Window <|-- SdlWindow
MultiscopicView - MultiscopicMultiplexer
note top of MultiscopicMultiplexer : MultiscopicView lets a MultiscopicMultiplexer set up\ntargets for the left and right eye rendering.
MultiscopicMultiplexer <|-- LinearAnaglyphsMultiplexer
MultiscopicMultiplexer <|-- QuadBufferMultiplexer
@enduml

Since shaders are tightly coupled with the C++ code together with which they are used, their code reside within their respective class, in string literals. These string literals are specified in the form `std::string code = R"lang=glsl(` so that the editor may detect the language to provide syntax highlighting and automatic indentation. To get language support in the string literals with Emacs, use `polymode` (tested with version 20190624.1927) with the following code in your `.emacs` file:

~~~~~~~~~~~~~{.el}
(require 'polymode)

(define-auto-innermode poly-c++-string-literals-lang-code-innermode
  :mode-matcher (cons "R\"lang=\\([^(\n]*\\)(" 1)
  :head-matcher "[^a-zA-Z0-9]R\"lang=[^(\n]*("
  :tail-matcher ")lang=[^\" \n]*\""
  :head-mode 'host
  :tail-mode 'host)

(define-polymode poly-c++-mode
  :hostmode 'poly-c++-hostmode
  :innermodes '(poly-c++-string-literals-lang-code-innermode))
~~~~~~~~~~~~~


## gmTouch

The *touch* module provides touch handling functionality for the implementation of touch screen or touch table interaction.

Required dependencies:

 - gmMisc

 - Eigen3 (at least version 3.3)

Optional dependencies:

 - SDL2, for SDL touch events support
 - TUIO, for TUIO touch events support
 - OpenSceneGraph, for OSG camera support (3D touch)


## gmMisc

The miscellaneous module (*misc*) contains functionality that cannot be categorized in a good way that would not result in one module for each class.

Optional dependencies:

 - Eigen3 (at least version 3.3), for end-fitting estimators (EFFOAW and EFHOAW)
