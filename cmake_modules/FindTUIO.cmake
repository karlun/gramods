# - Try to find TUIO
# Once done this will define
#  TUIO_FOUND - System has TUIO
#  TUIO_INCLUDE_DIRS - The TUIO include directories
#  TUIO_LIBRARIES - The libraries needed to use TUIO
#  TUIO_DEFINITIONS - Compiler switches required for using TUIO

find_package(PkgConfig)
pkg_check_modules(PC_LIBXML QUIET libxml-2.0)
set(TUIO_DEFINITIONS ${PC_LIBXML_CFLAGS_OTHER})

find_path(TUIO_INCLUDE_DIR TuioListener.h)
find_path(OSCPACK_INCLUDE_DIR ip/PacketListener.h)

find_library(TUIO_LIBRARY NAMES tuio libtuio TUIO libTUIO)

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set TUIO_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(TUIO  DEFAULT_MSG
                                  TUIO_LIBRARY TUIO_INCLUDE_DIR OSCPACK_INCLUDE_DIR)

mark_as_advanced(TUIO_INCLUDE_DIR OSCPACK_INCLUDE_DIR TUIO_LIBRARY)

set(TUIO_LIBRARIES ${TUIO_LIBRARY} )
set(TUIO_INCLUDE_DIRS ${TUIO_INCLUDE_DIR} ${OSCPACK_INCLUDE_DIR})
