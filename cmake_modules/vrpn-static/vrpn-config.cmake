# Final resort if no "real" vrpn-config.cmake file is available

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

# VRPN client library
ADD_LIBRARY(VRPN::Client STATIC IMPORTED)
IF (VRPN_INCLUDE_DIR AND VRPN_LIBRARY)
  SET_TARGET_PROPERTIES(VRPN::Client PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${VRPN_INCLUDE_DIR}
    IMPORTED_LOCATION ${VRPN_LIBRARY})
ELSE()
  MESSAGE(SEND_ERROR "Could not set properties of target VRPN::Client - set VRPN_INCLUDE_DIR and VRPN_LIBRARY or unset VRPN_DIR")
ENDIF()

# VRPN server library
ADD_LIBRARY(VRPN::Server STATIC IMPORTED)
IF (VRPN_INCLUDE_DIR AND VRPN_SERVER_LIBRARY)
  SET_TARGET_PROPERTIES(VRPN::Server PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${VRPN_INCLUDE_DIR}
    IMPORTED_LOCATION ${VRPN_SERVER_LIBRARY})
ELSE()
  MESSAGE(SEND_ERROR "Could not set properties of target VRPN::Server - set VRPN_INCLUDE_DIR and VRPN_SERVER_LIBRARY or unset VRPN_DIR")
ENDIF()

# VRPN quat library
ADD_LIBRARY(VRPN::Quat STATIC IMPORTED)
IF (QUAT_INCLUDE_DIR AND QUAT_LIBRARY)
  SET_TARGET_PROPERTIES(VRPN::Quat PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${QUAT_INCLUDE_DIR}
    IMPORTED_LOCATION ${QUAT_LIBRARY})
ELSE()
  MESSAGE(SEND_ERROR "Could not set properties of target VRPN::Quat - set QUAT_INCLUDE_DIR and QUAT_LIBRARY or unset VRPN_DIR")
ENDIF()