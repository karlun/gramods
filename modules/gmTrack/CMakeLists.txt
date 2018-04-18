CMAKE_MINIMUM_REQUIRED(VERSION 2.8.3)


SET (MODULE_INCLUDE_DIRS CACHE INTERNAL "The list of include folders to search for the track module.")
SET (MODULE_LIBS CACHE INTERNAL "The list of libraries that the track module should link against.")

SET (MODULE_INCLUDE_DIRS ${MODULE_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/include)
SET (MODULE_INCLUDE_DIRS ${MODULE_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR}/include)

SET (MODULE_LIBS ${MODULE_LIBS} gmCore)

FIND_PACKAGE (Eigen3 3.3 REQUIRED NO_MODULE)
SET (MODULE_LIBS ${MODULE_LIBS} Eigen3::Eigen)

FIND_PACKAGE(VRPN)
IF(VRPN_FOUND)
  SET(HAVE_VRPN 1)
  OPTION(GM_ENABLE_VRPN "" ON)
  SET (MODULE_INCLUDE_DIRS ${MODULE_INCLUDE_DIRS} ${VRPN_INCLUDE_DIR})
  SET (MODULE_LIBS ${MODULE_LIBS} ${VRPN_LIBRARIES})
ENDIF(VRPN_FOUND)


CONFIGURE_FILE(
  ${CMAKE_CURRENT_SOURCE_DIR}/include/gmTrack/config_cmake.hh
  ${CMAKE_CURRENT_BINARY_DIR}/include/gmTrack/config.hh)


ADD_LIBRARY(gmTrack SHARED
  src/VRPNTracker.cpp
  )

TARGET_INCLUDE_DIRECTORIES(gmTrack PUBLIC ${MODULE_INCLUDE_DIRS})
TARGET_LINK_LIBRARIES(gmTrack ${MODULE_LIBS})

SET_PROPERTY(TARGET gmTrack PROPERTY CXX_STANDARD 14)