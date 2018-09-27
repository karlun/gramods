# - Try to find FreeImage
# Once done this will define
#  FreeImage_FOUND - System has FreeImage
#  FreeImage_INCLUDE_DIRS - The FreeImage include directories
#  FreeImage_LIBRARIES - The libraries needed to use FreeImage

find_path(FreeImage_INCLUDE_DIR FreeImage.h)
find_library(FreeImage_LIBRARY NAMES freeimage)

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set FreeImage_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(FreeImage  DEFAULT_MSG
                                  FreeImage_LIBRARY FreeImage_INCLUDE_DIR)

mark_as_advanced(FreeImage_INCLUDE_DIR FreeImage_LIBRARY)

set(FreeImage_LIBRARIES ${FreeImage_LIBRARY})
set(FreeImage_INCLUDE_DIRS ${FreeImage_INCLUDE_DIR})
