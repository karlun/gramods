CMAKE_MINIMUM_REQUIRED(VERSION 2.8.3)


SET (MODULE_INCLUDE_DIRS CACHE INTERNAL "The list of include folders to search for the network module.")
SET (MODULE_LIBS CACHE INTERNAL "The list of libraries that the network module should link against.")

SET (MODULE_INCLUDE_DIRS ${MODULE_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/include)
SET (MODULE_INCLUDE_DIRS ${MODULE_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR}/include)

SET (MODULE_LIBS ${MODULE_LIBS} gmCore)

# Boost ASIO (headers only)
FIND_PACKAGE(ASIO REQUIRED)
SET (MODULE_INCLUDE_DIRS ${MODULE_INCLUDE_DIRS} ${ASIO_INCLUDE_DIRS})


OPTION(gramods_ACTIVATE_ASIO_HANDLER_TRACKING
  "Activate ASIO handler tracking and debug output to the standard error stream."
  OFF)

CONFIGURE_FILE(
  ${CMAKE_CURRENT_SOURCE_DIR}/include/gmNetwork/config_cmake.hh
  ${CMAKE_CURRENT_BINARY_DIR}/include/gmNetwork/config.hh)


ADD_LIBRARY(gmNetwork SHARED
  src/ExecutionSynchronization.cpp
  src/PeersConnection.cpp
  src/Protocol.cpp
  )

TARGET_INCLUDE_DIRECTORIES(gmNetwork PUBLIC ${MODULE_INCLUDE_DIRS})
TARGET_LINK_LIBRARIES(gmNetwork ${MODULE_LIBS})

SET_PROPERTY(TARGET gmNetwork PROPERTY CXX_STANDARD 14)
