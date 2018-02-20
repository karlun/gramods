# gramods

Gramods, short for Graphics Modules, is a collection of weakly inter dependent and useful APIs for Virtual Reality and graphics cluster rendering.

## Repository Structure

The repository divides the software into modules and apps where the modules provide functionality that the apps put together into executables. All modules listed in the modules/ folder will be compiled into separate libraries, and all apps in the apps/ folder will be compiled into executables.

The CMakeLists.txt file will automatically generate a list of modules based on the folders found in the modules/ folder. CMake will make sure that all targets are compiled and linked in the right order, as long as dependencies are specified correctly. If new modules or apps are added after the initialization of the cmake cache, by a git pull or manually, this must be manually added to its list.

## Modules and Dependencies

### gmConfig

Utilities for loading other modules and configure them based on configuration files.

Requirements: TinyXML2

