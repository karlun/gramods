# Find the ASIO includes and library
#
#  ASIO_INCLUDE_DIRS   - where to find ASIO include files
#  ASIO_FOUND          - True if ASIO was found

FIND_PATH(ASIO_INCLUDE_DIRS asio.hpp
  /usr/include /usr/local/include
)

# handle the QUIETLY and REQUIRED arguments and set ASIO_FOUND to TRUE
# if all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(ASIO DEFAULT_MSG ASIO_INCLUDE_DIRS)
