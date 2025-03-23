CMAKE_MINIMUM_REQUIRED(VERSION 3.13)
CMAKE_POLICY(VERSION 3.13)


SET (PRIVATE_INCLUDE_DIRS CACHE INTERNAL "Includes *not* used in module header files")
SET (PUBLIC_LIBS CACHE INTERNAL "Libraries with headers used in module header files")
SET (PRIVATE_LIBS CACHE INTERNAL "Libraries *not* used in module header files.")
SET (INTERNAL_LIBS CACHE INTERNAL "Internal targets and dependencies with full paths.")

SET(gramods_PARTS
  ${gramods_PARTS} lib_gmSound PARENT_SCOPE)

LIST (APPEND PUBLIC_LIBS gmTrack)
LIST (APPEND INTERNAL_LIBS gmTrack_internal_deps)

LIST (APPEND PUBLIC_LIBS gmMisc)
LIST (APPEND INTERNAL_LIBS gmMisc_internal_deps)

IF (NOT gramods_ENABLE_Eigen3)
  SET(gramods_REQ_lib_gmSound "Eigen3" PARENT_SCOPE)
  RETURN()
ENDIF()

FIND_PACKAGE(OpenAL QUIET)
IF (OPENAL_FOUND)
  SET(HAVE_OpenAL 1)
  OPTION(gramods_ENABLE_OpenAL "Enable functionality that requires OpenAL" ON)
  IF (NOT gramods_ENABLE_OpenAL)
    SET(gramods_REQ_lib_gmSound "OpenAL" PARENT_SCOPE)
    RETURN()
  ENDIF ()
ELSE()
  SET(gramods_REQ_lib_gmSound "OpenAL" PARENT_SCOPE)
  RETURN()
ENDIF ()
LIST (APPEND PRIVATE_INCLUDE_DIRS ${OPENAL_INCLUDE_DIRS})
LIST (APPEND PRIVATE_LIBS ${OPENAL_LIBRARY})
LIST (APPEND gramods_FEAT_lib_gmSound "OpenAL")


CONFIGURE_FILE(
  ${CMAKE_CURRENT_SOURCE_DIR}/src/config_cmake.hh
  ${CMAKE_CURRENT_BINARY_DIR}/include/gmSound/config.hh)


INCLUDE(GetSourcesAndHeaders)
get_sources_and_headers()

LIST(APPEND HEADERS_MAGIC $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include/gmSound/config.hh>)
LIST(APPEND HEADERS_MAGIC $<INSTALL_INTERFACE:include/gmSound/config.hh>)

SOURCE_GROUP("headers" FILES ${HEADERS_ABSOLUTE})
SOURCE_GROUP("sources" FILES ${SOURCES})

ADD_LIBRARY(gmSound SHARED)
SET(gramods_FEAT_lib_gmSound ${gramods_FEAT_lib_gmSound} PARENT_SCOPE)
SET(gramods_DISA_lib_gmSound ${gramods_DISA_lib_gmSound} PARENT_SCOPE)
SET(gramods_MISS_lib_gmSound ${gramods_MISS_lib_gmSound} PARENT_SCOPE)

TARGET_INCLUDE_DIRECTORIES(gmSound
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
  $<INSTALL_INTERFACE:include>
  PRIVATE
  ${PRIVATE_INCLUDE_DIRS})
TARGET_SOURCES(gmSound
  PRIVATE ${SOURCES}
  PUBLIC ${HEADERS_MAGIC})
TARGET_LINK_LIBRARIES(gmSound
  PUBLIC ${PUBLIC_LIBS}
  PRIVATE ${PRIVATE_LIBS})

# Get absolute library paths of dependencies into apps and tests
ADD_LIBRARY(gmSound_internal_deps INTERFACE)
TARGET_LINK_LIBRARIES(gmSound_internal_deps
  INTERFACE ${PUBLIC_LIBS} ${PRIVATE_LIBS} ${INTERNAL_LIBS} gmSound)


SET_PROPERTY(TARGET gmSound PROPERTY CXX_STANDARD 23)
SET_PROPERTY(TARGET gmSound PROPERTY WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
SET_PROPERTY(TARGET gmSound PROPERTY PUBLIC_HEADER ${HEADERS_MAGIC})

INSTALL(TARGETS gmSound
  EXPORT Gramods-gmSound-targets
  ARCHIVE LIBRARY RUNTIME
  PUBLIC_HEADER DESTINATION include/gmSound)
INSTALL(EXPORT Gramods-gmSound-targets
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/gramods/cmake)

LIST(APPEND gramods_LIB_TARGET_FILES "$<TARGET_FILE:gmSound>")
SET(gramods_LIB_TARGET_FILES ${gramods_LIB_TARGET_FILES} PARENT_SCOPE)
