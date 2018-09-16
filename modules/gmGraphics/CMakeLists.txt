CMAKE_MINIMUM_REQUIRED(VERSION 2.8.3)


SET (MODULE_INCLUDE_DIRS CACHE INTERNAL "The list of include folders to search for the graphics module.")
SET (MODULE_LIBS CACHE INTERNAL "The list of libraries that the graphics module should link against.")

SET (MODULE_INCLUDE_DIRS ${MODULE_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/include)
SET (MODULE_INCLUDE_DIRS ${MODULE_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR}/include)


SET (MODULE_LIBS ${MODULE_LIBS} gmCore)
SET (MODULE_LIBS ${MODULE_LIBS} gmTypes)

FIND_PACKAGE(SDL2)
IF(SDL2_FOUND)
  SET(HAVE_SDL2 1)
  OPTION(gramods_ENABLE_SDL2 "Enable functionality that requires SDL2" ON)
  IF(gramods_ENABLE_SDL2)
    SET (MODULE_INCLUDE_DIRS ${MODULE_INCLUDE_DIRS} ${SDL2_INCLUDE_DIR})
    SET (MODULE_LIBS ${MODULE_LIBS} ${SDL2_LIBRARY})
  ENDIF(gramods_ENABLE_SDL2)
ENDIF(SDL2_FOUND)

FIND_PACKAGE (Eigen3 3.3 NO_MODULE)
IF (Eigen3_FOUND)
  SET(HAVE_Eigen3 1)
  OPTION(gramods_ENABLE_Eigen3 "Enable functionality that requires Eigen3" ON)
  IF (NOT gramods_ENABLE_Eigen3)
    MESSAGE (STATUS "Not including gmGraphics - requires Eigen3 (disabled)")
    RETURN()
  ENDIF()
ELSE()
  MESSAGE (STATUS "Not including gmGraphics - requires Eigen3")
  RETURN()
ENDIF()
SET (MODULE_LIBS ${MODULE_LIBS} Eigen3::Eigen)

FIND_PACKAGE (globjects)
IF (globjects_FOUND)
  SET(HAVE_globjects 1)
  OPTION(gramods_ENABLE_globjects "Enable functionality that requires globjects" ON)
  IF (NOT gramods_ENABLE_globjects)
    MESSAGE (STATUS "Not including gmGraphics - requires globjects (disabled)")
    RETURN()
  ENDIF()
ELSE()
  MESSAGE (STATUS "Not including gmGraphics - requires globjects")
  RETURN()
ENDIF()


CONFIGURE_FILE(
  ${CMAKE_CURRENT_SOURCE_DIR}/include/gmGraphics/config_cmake.hh
  ${CMAKE_CURRENT_BINARY_DIR}/include/gmGraphics/config.hh)


FILE(GLOB_RECURSE HEADERS include/*.hh)
FILE(GLOB_RECURSE SOURCES src/*.cpp)

SOURCE_GROUP("headers" FILES ${HEADERS})
SOURCE_GROUP("sources" FILES ${SOURCES})

ADD_LIBRARY(gmGraphics SHARED ${HEADERS} ${SOURCES})

TARGET_INCLUDE_DIRECTORIES(gmGraphics PUBLIC ${MODULE_INCLUDE_DIRS})
TARGET_LINK_LIBRARIES(gmGraphics ${MODULE_LIBS})

SET_PROPERTY(TARGET gmGraphics PROPERTY CXX_STANDARD 14)

INSTALL(TARGETS gmGraphics
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
INSTALL(DIRECTORY include/
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})