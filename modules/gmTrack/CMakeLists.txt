CMAKE_MINIMUM_REQUIRED(VERSION 3.13)
CMAKE_POLICY(VERSION 3.13)


SET (PRIVATE_INCLUDE_DIRS CACHE INTERNAL "The list of private include folders for the track module.")
SET (MODULE_LIBS CACHE INTERNAL "The list of libraries that the track module should link against.")


LIST (APPEND MODULE_LIBS gmCore)

FIND_PACKAGE (Eigen3 QUIET NO_MODULE)
IF (Eigen3_FOUND)
  SET(HAVE_Eigen3 1)
  OPTION(gramods_ENABLE_Eigen3 "Enable functionality that requires Eigen3" ON)
  IF (NOT gramods_ENABLE_Eigen3)
    MESSAGE (STATUS "Not including gmTrack - requires Eigen3 (disabled)")
    RETURN()
  ENDIF()
ELSE()
  MESSAGE (WARNING "Not including gmTrack - requires Eigen3")
  RETURN()
ENDIF()
LIST (APPEND MODULE_LIBS Eigen3::Eigen)

FIND_PACKAGE(VRPN)
IF(VRPN_FOUND)
  SET(HAVE_VRPN 1)
  OPTION(gramods_ENABLE_VRPN "Enable functionality that requires VRPN" ON)
  SET(HAVE_VRPN_server 1)
  OPTION(gramods_ENABLE_VRPN_server "Enable functionality that requires VRPN_server" ON)
  IF (gramods_ENABLE_VRPN OR gramods_ENABLE_VRPN_server)
    LIST (APPEND PRIVATE_INCLUDE_DIRS ${VRPN_INCLUDE_DIR})
    LIST (APPEND MODULE_LIBS ${VRPN_LIBRARIES})
  ENDIF(gramods_ENABLE_VRPN OR gramods_ENABLE_VRPN_server)
ENDIF(VRPN_FOUND)

FIND_PACKAGE(OpenCV QUIET OPTIONAL_COMPONENTS aruco)
IF(OpenCV_FOUND AND NOT OPENCV_ARUCO_FOUND)
  MESSAGE(STATUS "Found OpenCV without aruco module")
ENDIF()
IF(OPENCV_ARUCO_FOUND)
  IF (NOT OpenCV_VERSION_MAJOR EQUAL 4)
    MESSAGE(WARNING "OpenCV version is not 4 - errors may occur")
  ENDIF()
  SET(HAVE_aruco 1)
  OPTION(gramods_ENABLE_aruco "Enable functionality that requires aruco" ON)
  IF (gramods_ENABLE_aruco)
    LIST (APPEND MODULE_LIBS ${OpenCV_LIBS})
  ENDIF()
ENDIF()

CONFIGURE_FILE(
  ${CMAKE_CURRENT_SOURCE_DIR}/src/config_cmake.hh
  ${CMAKE_CURRENT_BINARY_DIR}/include/gmTrack/config.hh)


INCLUDE(GetSourcesAndHeaders)
get_sources_and_headers()

SOURCE_GROUP("headers" FILES ${HEADERS_ABSOLUTE})
SOURCE_GROUP("sources" FILES ${SOURCES})

ADD_LIBRARY(gmTrack SHARED)

TARGET_INCLUDE_DIRECTORIES(gmTrack
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
  $<INSTALL_INTERFACE:include>
  PRIVATE
  ${PRIVATE_INCLUDE_DIRS})
TARGET_SOURCES(gmTrack
  PRIVATE ${SOURCES}
  PUBLIC ${HEADERS_MAGIC}
  )
TARGET_LINK_LIBRARIES(gmTrack ${MODULE_LIBS})

INCLUDE(GenerateExportHeader)
GENERATE_EXPORT_HEADER(gmTrack
  EXPORT_FILE_NAME include/gmTrack/export.hh
  EXPORT_MACRO_NAME gmTrack_API)

SET_PROPERTY(TARGET gmTrack PROPERTY CXX_STANDARD 17)
SET_PROPERTY(TARGET gmTrack PROPERTY WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
SET_PROPERTY(TARGET gmTrack PROPERTY PUBLIC_HEADER ${HEADERS_MAGIC})

INSTALL(TARGETS gmTrack
  EXPORT Gramods-gmTrack-targets
  ARCHIVE LIBRARY RUNTIME
  PUBLIC_HEADER DESTINATION include/gmTrack)
INSTALL(EXPORT Gramods-gmTrack-targets
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/gramods/cmake)

INSTALL(DIRECTORY resources/
  DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/gramods/gmTrack/resources)
INSTALL(DIRECTORY config/
  DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/gramods/gmTrack/config)

SET(gramods_LIB_TARGET_FILES ${gramods_LIB_TARGET_FILES} "$<TARGET_FILE:gmTrack>" PARENT_SCOPE)
