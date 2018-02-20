# - Find VRPN
# Find the native VRPN headers and libraries.
#
#  VRPN_INCLUDE_DIR -  where to find header files.
#  VRPN_LIBRARIES    - List of libraries when using VRPN.
#  VRPN_FOUND        - True if VRPN found.


# Look for the header file.
FIND_PATH(VRPN_INCLUDE_DIR NAMES vrpn_Tracker.h
                             PATHS $ENV{H3D_EXTERNAL_ROOT}/include
                                   ../../External/include )
MARK_AS_ADVANCED(VRPN_INCLUDE_DIR)

FIND_PATH(QUAT_INCLUDE_DIR NAMES quat.h
                             PATHS $ENV{H3D_EXTERNAL_ROOT}/include
                                   ../../External/include )
MARK_AS_ADVANCED(QUAT_INCLUDE_DIR)

# Look for the library.
FIND_LIBRARY(VRPN_LIBRARY NAMES vrpn
                            PATHS $ENV{H3D_EXTERNAL_ROOT}/lib
                                  ../../External/lib )
MARK_AS_ADVANCED(VRPN_LIBRARY)

FIND_LIBRARY(QUAT_LIBRARY NAMES quat
                            PATHS $ENV{H3D_EXTERNAL_ROOT}/lib
                                  ../../External/lib )
MARK_AS_ADVANCED(QUAT_LIBRARY)

# Copy the results to the output variables.
IF(VRPN_INCLUDE_DIR AND VRPN_LIBRARY AND QUAT_INCLUDE_DIR AND QUAT_LIBRARY)
  SET(VRPN_FOUND 1)

  SET(VRPN_LIBRARIES ${VRPN_LIBRARY})
  SET(VRPN_INCLUDE_DIR ${VRPN_INCLUDE_DIR})

  SET(VRPN_LIBRARIES ${VRPN_LIBRARIES} ${QUAT_LIBRARY})
  SET(VRPN_INCLUDE_DIR ${VRPN_INCLUDE_DIR} ${QUAT_INCLUDE_DIR})

ELSE(VRPN_INCLUDE_DIR AND VRPN_LIBRARY AND QUAT_INCLUDE_DIR AND QUAT_LIBRARY)
  SET(VRPN_FOUND 0)
  SET(VRPN_LIBRARIES)
  SET(VRPN_INCLUDE_DIR)
ENDIF(VRPN_INCLUDE_DIR AND VRPN_LIBRARY AND QUAT_INCLUDE_DIR AND QUAT_LIBRARY)

# Report the results.
IF(NOT VRPN_FOUND)
  SET(VRPN_DIR_MESSAGE
    "VRPN was not found.")
  IF(VRPN_FIND_REQUIRED)
    MESSAGE(FATAL_ERROR "${VRPN_DIR_MESSAGE}")
  ELSEIF(NOT VRPN_FIND_QUIETLY)
    MESSAGE(STATUS "${VRPN_DIR_MESSAGE}")
  ENDIF(VRPN_FIND_REQUIRED)
ENDIF(NOT VRPN_FOUND)
