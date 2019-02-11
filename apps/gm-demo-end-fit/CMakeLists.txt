CMAKE_MINIMUM_REQUIRED(VERSION 2.8.3)


SET (APP_INCLUDE_DIRS CACHE INTERNAL "The list of include folders to search for the app.")
SET (APP_LIBS CACHE INTERNAL "The list of libraries that the app should link against.")

SET (APP_LIBS ${APP_LIBS} gmMisc)


FIND_PACKAGE(SDL2)
IF(SDL2_FOUND)
  SET(HAVE_SDL2 1)
  OPTION(gramods_ENABLE_SDL2 "Enable functionality that requires SDL2" ON)
  IF(NOT gramods_ENABLE_SDL2)
    MESSAGE (STATUS "Not including gm-demo-end-fit - requires SDL2 (disabled)")
    RETURN()
  ENDIF()
ELSE()
  MESSAGE (STATUS "Not including gm-demo-end-fit - requires SDL2")
  RETURN()
ENDIF()
SET (APP_INCLUDE_DIRS ${APP_INCLUDE_DIRS} ${SDL2_INCLUDE_DIR})
SET (APP_LIBS ${APP_LIBS} ${SDL2_LIBRARY})


FILE(GLOB_RECURSE HEADERS include/*.hh)
FILE(GLOB_RECURSE SOURCES src/*.cpp)

SOURCE_GROUP("headers" FILES ${HEADERS})
SOURCE_GROUP("sources" FILES ${SOURCES})

ADD_EXECUTABLE(gm-demo-end-fit ${HEADERS} ${SOURCES})

TARGET_INCLUDE_DIRECTORIES(gm-demo-end-fit PUBLIC ${APP_INCLUDE_DIRS})
TARGET_LINK_LIBRARIES(gm-demo-end-fit ${APP_LIBS})

SET_PROPERTY(TARGET gm-demo-end-fit PROPERTY CXX_STANDARD 14)