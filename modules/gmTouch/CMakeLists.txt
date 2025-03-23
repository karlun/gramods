CMAKE_MINIMUM_REQUIRED(VERSION 3.13)
CMAKE_POLICY(VERSION 3.13)


SET (PRIVATE_INCLUDE_DIRS CACHE INTERNAL "Includes *not* used in module header files")
SET (PUBLIC_LIBS CACHE INTERNAL "Libraries with headers used in module header files")
SET (PRIVATE_LIBS CACHE INTERNAL "Libraries *not* used in module header files.")
SET (INTERNAL_LIBS CACHE INTERNAL "Internal targets and dependencies with full paths.")

SET(gramods_PARTS
  ${gramods_PARTS} lib_gmTouch PARENT_SCOPE)

LIST (APPEND PUBLIC_LIBS gmMisc)
LIST (APPEND INTERNAL_LIBS gmMisc_internal_deps)

IF (NOT gramods_ENABLE_Eigen3)
  SET(gramods_REQ_lib_gmTouch "Eigen3" PARENT_SCOPE)
  RETURN()
ENDIF()

FIND_PACKAGE(OpenSceneGraph QUIET)
IF (OPENSCENEGRAPH_FOUND)
  SET(HAVE_OpenSceneGraph 1)
  OPTION(gramods_ENABLE_OpenSceneGraph "Enable functionality that requires OpenSceneGraph" ON)
  IF (gramods_ENABLE_OpenSceneGraph)
    LIST (APPEND PRIVATE_INCLUDE_DIRS ${OPENSCENEGRAPH_INCLUDE_DIRS})
    LIST (APPEND PUBLIC_LIBS ${OPENSCENEGRAPH_LIBRARIES})
    LIST(APPEND gramods_FEAT_lib_gmTouch OpenSceneGraph)
  ELSE()
    LIST(APPEND gramods_DISA_lib_gmTouch OpenSceneGraph)
  ENDIF ()
ELSE()
  LIST(APPEND gramods_MISS_lib_gmTouch OpenSceneGraph)
ENDIF ()

FIND_PACKAGE(TUIO QUIET)
IF (TUIO_FOUND)
  SET(HAVE_TUIO 1)
  OPTION(gramods_ENABLE_TUIO "Enable functionality that requires TUIO" ON)
  IF (gramods_ENABLE_TUIO)
    LIST (APPEND PRIVATE_INCLUDE_DIRS ${TUIO_INCLUDE_DIRS})
    LIST (APPEND PRIVATE_LIBS ${TUIO_LIBRARY})
    LIST(APPEND gramods_FEAT_lib_gmTouch TUIO)
  ELSE()
    LIST(APPEND gramods_DISA_lib_gmTouch TUIO)
  ENDIF ()
ELSE()
  LIST(APPEND gramods_MISS_lib_gmTouch TUIO)
ENDIF ()


CONFIGURE_FILE(
  ${CMAKE_CURRENT_SOURCE_DIR}/src/config_cmake.hh
  ${CMAKE_CURRENT_BINARY_DIR}/include/gmTouch/config.hh)


INCLUDE(GetSourcesAndHeaders)
get_sources_and_headers()

LIST(APPEND HEADERS_MAGIC $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include/gmTouch/config.hh>)
LIST(APPEND HEADERS_MAGIC $<INSTALL_INTERFACE:include/gmTouch/config.hh>)

SOURCE_GROUP("headers" FILES ${HEADERS_ABSOLUTE})
SOURCE_GROUP("sources" FILES ${SOURCES})

ADD_LIBRARY(gmTouch SHARED)
SET(gramods_FEAT_lib_gmTouch ${gramods_FEAT_lib_gmTouch} PARENT_SCOPE)
SET(gramods_DISA_lib_gmTouch ${gramods_DISA_lib_gmTouch} PARENT_SCOPE)
SET(gramods_MISS_lib_gmTouch ${gramods_MISS_lib_gmTouch} PARENT_SCOPE)

TARGET_INCLUDE_DIRECTORIES(gmTouch
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
  $<INSTALL_INTERFACE:include>
  PRIVATE
  ${PRIVATE_INCLUDE_DIRS})
TARGET_SOURCES(gmTouch
  PRIVATE ${SOURCES}
  PUBLIC ${HEADERS_MAGIC})
TARGET_LINK_LIBRARIES(gmTouch
  PUBLIC ${PUBLIC_LIBS}
  PRIVATE ${PRIVATE_LIBS})

# Get absolute library paths of dependencies into apps and tests
ADD_LIBRARY(gmTouch_internal_deps INTERFACE)
TARGET_LINK_LIBRARIES(gmTouch_internal_deps
  INTERFACE ${PUBLIC_LIBS} ${PRIVATE_LIBS} ${INTERNAL_LIBS} gmTouch)


SET_PROPERTY(TARGET gmTouch PROPERTY CXX_STANDARD 17)
SET_PROPERTY(TARGET gmTouch PROPERTY WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
SET_PROPERTY(TARGET gmTouch PROPERTY PUBLIC_HEADER ${HEADERS_MAGIC})

INSTALL(TARGETS gmTouch
  EXPORT Gramods-gmTouch-targets
  ARCHIVE LIBRARY RUNTIME
  PUBLIC_HEADER DESTINATION include/gmTouch)
INSTALL(EXPORT Gramods-gmTouch-targets
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/gramods/cmake)

LIST(APPEND gramods_LIB_TARGET_FILES "$<TARGET_FILE:gmTouch>")
SET(gramods_LIB_TARGET_FILES ${gramods_LIB_TARGET_FILES} PARENT_SCOPE)
