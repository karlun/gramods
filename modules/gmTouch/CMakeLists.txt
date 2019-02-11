CMAKE_MINIMUM_REQUIRED(VERSION 2.8.3)


SET (MODULE_INCLUDE_DIRS CACHE INTERNAL "The list of include folders to search for the touch module.")
SET (MODULE_LIBS CACHE INTERNAL "The list of libraries that the touch module should link against.")

SET (MODULE_INCLUDE_DIRS ${MODULE_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/include)
SET (MODULE_INCLUDE_DIRS ${MODULE_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR}/include)


SET (MODULE_LIBS ${MODULE_LIBS} gmMisc)

FIND_PACKAGE (Eigen3 QUIET NO_MODULE)
IF (Eigen3_FOUND)
  SET(HAVE_Eigen3 1)
  OPTION(gramods_ENABLE_Eigen3 "" ON)
  IF (NOT gramods_ENABLE_Eigen3)
    MESSAGE (STATUS "Not including gmTouch - requires Eigen3 (disabled)")
    RETURN()
  ENDIF()
ELSE()
  MESSAGE (STATUS "Not including gmTouch - requires Eigen3")
  RETURN()
ENDIF()
SET (MODULE_LIBS ${MODULE_LIBS} Eigen3::Eigen)

FIND_PACKAGE(OpenSceneGraph)
IF (OPENSCENEGRAPH_FOUND)
  SET(HAVE_OpenSceneGraph 1)
  OPTION(gramods_ENABLE_OpenSceneGraph "" ON)
  IF (gramods_ENABLE_OpenSceneGraph)
    SET (MODULE_INCLUDE_DIRS ${MODULE_INCLUDE_DIRS} ${OPENSCENEGRAPH_INCLUDE_DIRS})
    SET (MODULE_LIBS ${MODULE_LIBS} ${OPENSCENEGRAPH_LIBRARIES})
  ENDIF ()
ENDIF ()

FIND_PACKAGE(SDL2)
IF (SDL2_FOUND)
  SET(HAVE_SDL2 1)
  OPTION(gramods_ENABLE_SDL2 "" ON)
  IF (gramods_ENABLE_SDL2)
    SET (MODULE_INCLUDE_DIRS ${MODULE_INCLUDE_DIRS} ${SDL2_INCLUDE_DIR})
    SET (MODULE_LIBS ${MODULE_LIBS} ${SDL2_LIBRARY})
  ENDIF ()
ENDIF ()

FIND_PACKAGE(TUIO)
IF (TUIO_FOUND)
  SET(HAVE_TUIO 1)
  OPTION(gramods_ENABLE_TUIO "" ON)
  IF (gramods_ENABLE_TUIO)
    SET (MODULE_INCLUDE_DIRS ${MODULE_INCLUDE_DIRS} ${TUIO_INCLUDE_DIRS})
    SET (MODULE_LIBS ${MODULE_LIBS} ${TUIO_LIBRARY})
  ENDIF ()
ENDIF ()


CONFIGURE_FILE(
  ${CMAKE_CURRENT_SOURCE_DIR}/include/gmTouch/config_cmake.hh
  ${CMAKE_CURRENT_BINARY_DIR}/include/gmTouch/config.hh)


FILE(GLOB_RECURSE HEADERS include/*.hh)
FILE(GLOB_RECURSE SOURCES src/*.cpp)

SOURCE_GROUP("headers" FILES ${HEADERS})

ADD_LIBRARY(gmTouch ${HEADERS} ${SOURCES})

TARGET_INCLUDE_DIRECTORIES(gmTouch PUBLIC ${MODULE_INCLUDE_DIRS})
TARGET_LINK_LIBRARIES(gmTouch ${MODULE_LIBS})

SET_PROPERTY(TARGET gmTouch PROPERTY CXX_STANDARD 14)