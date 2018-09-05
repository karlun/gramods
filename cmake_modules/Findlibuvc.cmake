# - Try to find libuvc
# Once done this will define
#  libuvc_FOUND - System has libuvc
#  libuvc_INCLUDE_DIRS - The libuvc include directories
#  libuvc_LIBRARIES - The libraries needed to use libuvc
#  libuvc_DEFINITIONS - Compiler switches required for using libuvc

find_package(PkgConfig)
pkg_check_modules(PC_LIBXML QUIET libxml-2.0)
set(libuvc_DEFINITIONS ${PC_LIBXML_CFLAGS_OTHER})

find_path(libuvc_INCLUDE_DIR libuvc_config.h)

find_library(libuvc_LIBRARY NAMES libuvc)

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set libuvc_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(libuvc  DEFAULT_MSG
                                  libuvc_LIBRARY libuvc_INCLUDE_DIR)

mark_as_advanced(libuvc_INCLUDE_DIR libuvc_LIBRARY)

set(libuvc_LIBRARIES ${libuvc_LIBRARY} )
set(libuvc_INCLUDE_DIRS ${libuvc_INCLUDE_DIR})
