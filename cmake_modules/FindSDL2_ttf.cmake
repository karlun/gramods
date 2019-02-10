# - Try to find SDL2_ttf
# Once done this will define
#  SDL2_ttf_FOUND - System has SDL2_ttf
#  SDL2_ttf_INCLUDE_DIRS - The SDL2_ttf include directories
#  SDL2_ttf_LIBRARIES - The libraries needed to use SDL2_ttf

find_path(SDL2_ttf_INCLUDE_DIR SDL2/SDL_ttf.h)
find_library(SDL2_ttf_LIBRARY NAMES SDL2_ttf)

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set SDL2_ttf_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(SDL2_ttf  DEFAULT_MSG
                                  SDL2_ttf_LIBRARY SDL2_ttf_INCLUDE_DIR)

mark_as_advanced(SDL2_ttf_INCLUDE_DIR SDL2_ttf_LIBRARY)

set(SDL2_ttf_LIBRARIES ${SDL2_ttf_LIBRARY} )
set(SDL2_ttf_INCLUDE_DIRS ${SDL2_ttf_INCLUDE_DIR})
