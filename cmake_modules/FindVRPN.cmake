# - Find VRPN
# Find the native VRPN headers and libraries.
#
#  VRPN_INCLUDE_DIR  -  where to find header files.
#  VRPN_LIBRARIES    - List of libraries when using VRPN.
#  VRPN_FOUND        - True if VRPN found, client only.
#  VRPN_server_FOUND - True if VRPN server library was found.


# Look for the header file.
FIND_PATH(VRPN_INCLUDE_DIR NAMES vrpn_Tracker.h PATHS ../deps/include /usr/include /usr/local/include)
MARK_AS_ADVANCED(VRPN_INCLUDE_DIR)

FIND_PATH(QUAT_INCLUDE_DIR NAMES quat.h PATHS ../deps/include /usr/include /usr/local/include)
MARK_AS_ADVANCED(QUAT_INCLUDE_DIR)

# Look for the library.
FIND_LIBRARY(VRPN_LIBRARY NAMES vrpn PATHS ../deps /usr/lib /usr/local/lib)
MARK_AS_ADVANCED(VRPN_LIBRARY)

FIND_LIBRARY(VRPN_SERVER_LIBRARY NAMES vrpnserver PATHS ../deps /usr/lib /usr/local/lib)
MARK_AS_ADVANCED(VRPN_SERVER_LIBRARY)

FIND_LIBRARY(QUAT_LIBRARY NAMES quat PATHS ../deps /usr/lib /usr/local/lib)
MARK_AS_ADVANCED(QUAT_LIBRARY)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(VRPN REQUIRED_VARS
  VRPN_INCLUDE_DIR VRPN_LIBRARY
  QUAT_INCLUDE_DIR QUAT_LIBRARY)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(VRPN_server REQUIRED_VARS
  VRPN_INCLUDE_DIR VRPN_LIBRARY
  VRPN_SERVER_LIBRARY
  QUAT_INCLUDE_DIR QUAT_LIBRARY)

IF (VRPN_FOUND)
  SET(VRPN_LIBRARIES ${VRPN_LIBRARY} ${QUAT_LIBRARY})
  SET(VRPN_INCLUDE_DIR ${VRPN_INCLUDE_DIR} ${QUAT_INCLUDE_DIR})

  IF (VRPN_server_FOUND)
    SET(VRPN_LIBRARIES ${VRPN_SERVER_LIBRARY} ${QUAT_LIBRARY})
  ENDIF()
ELSE()
  SET(VRPN_INCLUDE_DIR)
ENDIF()
