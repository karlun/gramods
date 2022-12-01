CMAKE_MINIMUM_REQUIRED(VERSION 3.13)
CMAKE_POLICY(VERSION 3.13)


SET (PRIVATE_INCLUDE_DIRS CACHE INTERNAL "Includes *not* used in module header files")
SET (PUBLIC_LIBS CACHE INTERNAL "Libraries with headers used in module header files")
SET (PRIVATE_LIBS CACHE INTERNAL "Libraries *not* used in module header files.")
SET (INTERNAL_LIBS CACHE INTERNAL "Internal targets and dependencies with full paths.")


LIST (APPEND PUBLIC_LIBS gmCore)
LIST (APPEND INTERNAL_LIBS gmCore_internal_deps)

# Boost ASIO (headers only)
FIND_PACKAGE (ASIO)
IF (ASIO_FOUND)

  SET(HAVE_ASIO 1)
  LIST(APPEND gramods_DOCS_DEFINES HAVE_ASIO)
  OPTION(gramods_ENABLE_ASIO "Enable functionality that requires ASIO" ON)

  IF (NOT gramods_ENABLE_ASIO)
    MESSAGE (STATUS "Not including gmNetwork - requires ASIO (disabled)")
    RETURN()
  ENDIF()
  LIST(APPEND gramods_DOCS_DEFINES gramods_ENABLE_ASIO)
ELSE()
  MESSAGE (STATUS "Not including gmNetwork - requires ASIO")
  RETURN()
ENDIF()
LIST (APPEND PRIVATE_INCLUDE_DIRS ${ASIO_INCLUDE_DIRS})


SET(gramods_DOCS_DEFINES ${gramods_DOCS_DEFINES} PARENT_SCOPE)

OPTION(gramods_ACTIVATE_ASIO_HANDLER_TRACKING
  "Activate ASIO handler tracking and debug output to the standard error stream."
  OFF)

INCLUDE(WinVer)
get_WIN32_WINNT(_WIN32_WINNT)
SET(_WIN32_WINNT "${_WIN32_WINNT}")

CONFIGURE_FILE(
  ${CMAKE_CURRENT_SOURCE_DIR}/src/config_cmake.hh
  ${CMAKE_CURRENT_BINARY_DIR}/include/gmNetwork/config.hh)


INCLUDE(GetSourcesAndHeaders)
get_sources_and_headers()

LIST(APPEND HEADERS_MAGIC $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include/gmNetwork/config.hh>)
LIST(APPEND HEADERS_MAGIC $<INSTALL_INTERFACE:include/gmNetwork/config.hh>)

SOURCE_GROUP("headers" FILES ${HEADERS_ABSOLUTE})
SOURCE_GROUP("sources" FILES ${SOURCES})

ADD_LIBRARY(gmNetwork SHARED)

TARGET_INCLUDE_DIRECTORIES(gmNetwork
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
  $<INSTALL_INTERFACE:include>
  PRIVATE
  ${PRIVATE_INCLUDE_DIRS})
TARGET_SOURCES(gmNetwork
  PRIVATE ${SOURCES}
  PUBLIC ${HEADERS_MAGIC})
TARGET_LINK_LIBRARIES(gmNetwork
  PUBLIC ${PUBLIC_LIBS}
  PRIVATE ${PRIVATE_LIBS})

# Get absolute library paths of dependencies into apps and tests
ADD_LIBRARY(gmNetwork_internal_deps INTERFACE)
TARGET_LINK_LIBRARIES(gmNetwork_internal_deps
  INTERFACE ${PUBLIC_LIBS} ${PRIVATE_LIBS} ${INTERNAL_LIBS} gmNetwork)

SET_PROPERTY(TARGET gmNetwork PROPERTY CXX_STANDARD 17)
SET_PROPERTY(TARGET gmNetwork PROPERTY WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
SET_PROPERTY(TARGET gmNetwork PROPERTY PUBLIC_HEADER ${HEADERS_MAGIC})

INSTALL(TARGETS gmNetwork
  EXPORT Gramods-gmNetwork-targets
  ARCHIVE LIBRARY RUNTIME
  PUBLIC_HEADER DESTINATION include/gmNetwork)
INSTALL(EXPORT Gramods-gmNetwork-targets
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/gramods/cmake)

LIST(APPEND gramods_LIB_TARGET_FILES "$<TARGET_FILE:gmNetwork>")
SET(gramods_LIB_TARGET_FILES ${gramods_LIB_TARGET_FILES} PARENT_SCOPE)
