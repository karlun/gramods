CMAKE_MINIMUM_REQUIRED(VERSION 3.13)
CMAKE_POLICY(VERSION 3.13)


SET (MODULE_INCLUDE_DIRS CACHE INTERNAL "The list of include folders for the touch module.")
SET (PUBLIC_LIBS CACHE INTERNAL "The list of public libraries that the touch module should link against.")
SET (PRIVATE_LIBS CACHE INTERNAL "The list of private libraries that the touch module should link against.")


LIST (APPEND PUBLIC_LIBS gmMisc)

FIND_PACKAGE (Eigen3 QUIET NO_MODULE)
IF (Eigen3_FOUND)
  SET(HAVE_Eigen3 1)
  OPTION(gramods_ENABLE_Eigen3 "Enable functionality that requires Eigen3" ON)
  IF (NOT gramods_ENABLE_Eigen3)
    MESSAGE (STATUS "Not including gmTouch - requires Eigen3 (disabled)")
    RETURN()
  ENDIF()
ELSE()
  MESSAGE (STATUS "Not including gmTouch - requires Eigen3")
  RETURN()
ENDIF()
LIST (APPEND PUBLIC_LIBS Eigen3::Eigen)

FIND_PACKAGE(OpenSceneGraph)
IF (OPENSCENEGRAPH_FOUND)
  SET(HAVE_OpenSceneGraph 1)
  OPTION(gramods_ENABLE_OpenSceneGraph "Enable functionality that requires OpenSceneGraph" ON)
  IF (gramods_ENABLE_OpenSceneGraph)
    LIST (APPEND MODULE_INCLUDE_DIRS ${OPENSCENEGRAPH_INCLUDE_DIRS})
    LIST (APPEND PUBLIC_LIBS ${OPENSCENEGRAPH_LIBRARIES})
  ENDIF ()
ENDIF ()

FIND_PACKAGE(SDL2)
IF (SDL2_FOUND)
  SET(HAVE_SDL2 1)
  OPTION(gramods_ENABLE_SDL2 "Enable functionality that requires SDL2" ON)
  IF (gramods_ENABLE_SDL2)
    LIST (APPEND PUBLIC_LIBS SDL2::SDL2)
  ENDIF ()
ENDIF ()

FIND_PACKAGE(TUIO)
IF (TUIO_FOUND)
  SET(HAVE_TUIO 1)
  OPTION(gramods_ENABLE_TUIO "Enable functionality that requires TUIO" ON)
  IF (gramods_ENABLE_TUIO)
    LIST (APPEND MODULE_INCLUDE_DIRS ${TUIO_INCLUDE_DIRS})
    LIST (APPEND PRIVATE_LIBS ${TUIO_LIBRARY})
  ENDIF ()
ENDIF ()


CONFIGURE_FILE(
  ${CMAKE_CURRENT_SOURCE_DIR}/src/config_cmake.hh
  ${CMAKE_CURRENT_BINARY_DIR}/include/gmTouch/config.hh)


INCLUDE(MakeDependenciesRelocatable)
make_dependencies_relocatable(PUBLIC_LIBS_MAGIC ${PUBLIC_LIBS})

INCLUDE(GetSourcesAndHeaders)
get_sources_and_headers()

LIST(APPEND HEADERS_MAGIC $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include/gmTouch/config.hh>)
LIST(APPEND HEADERS_MAGIC $<INSTALL_INTERFACE:include/gmTouch/config.hh>)

SOURCE_GROUP("headers" FILES ${HEADERS_ABSOLUTE})
SOURCE_GROUP("sources" FILES ${SOURCES})

ADD_LIBRARY(gmTouch SHARED)

TARGET_INCLUDE_DIRECTORIES(gmTouch
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
  $<INSTALL_INTERFACE:include>
  PRIVATE
  ${MODULE_INCLUDE_DIRS})
TARGET_SOURCES(gmTouch
  PRIVATE ${SOURCES}
  PUBLIC ${HEADERS_MAGIC}
  )
TARGET_LINK_LIBRARIES(gmTouch
  PUBLIC ${PUBLIC_LIBS_MAGIC}
  PRIVATE ${PRIVATE_LIBS})

# Get absolute library paths of dependencies into apps and tests
ADD_LIBRARY(gmTouch_internal_deps INTERFACE)
TARGET_INCLUDE_DIRECTORIES(gmTouch_internal_deps
  INTERFACE ${MODULE_INCLUDE_DIRS})
TARGET_LINK_LIBRARIES(gmTouch_internal_deps
  INTERFACE ${PUBLIC_LIBS} ${PRIVATE_LIBS} gmTouch)


SET_PROPERTY(TARGET gmTouch PROPERTY CXX_STANDARD 14)
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
