CMAKE_MINIMUM_REQUIRED(VERSION 3.10)
CMAKE_POLICY(VERSION 3.10)


SET (APP_INCLUDE_DIRS CACHE INTERNAL "The list of include folders to search for the app.")
SET (APP_LIBS CACHE INTERNAL "The list of libraries that the app should link against.")


IF (NOT TARGET gmMisc)
  MESSAGE (STATUS "Not including gm-demo-end-fit - requires gmMisc")
  RETURN()
ENDIF ()
LIST (APPEND APP_LIBS gmMisc)

IF (NOT gramods_ENABLE_Eigen3)
  MESSAGE (STATUS "Not including gm-demo-end-fit - requires Eigen3")
  RETURN()
ENDIF()

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
LIST (APPEND APP_LIBS SDL2::SDL2)


FILE(GLOB_RECURSE HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hh)
FILE(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

SOURCE_GROUP("headers" FILES ${HEADERS})
SOURCE_GROUP("sources" FILES ${SOURCES})

ADD_EXECUTABLE(gm-demo-end-fit ${HEADERS} ${SOURCES})

TARGET_INCLUDE_DIRECTORIES(gm-demo-end-fit PUBLIC ${APP_INCLUDE_DIRS})
TARGET_LINK_LIBRARIES(gm-demo-end-fit ${APP_LIBS})

SET_PROPERTY(TARGET gm-demo-end-fit PROPERTY CXX_STANDARD 17)
SET_PROPERTY(TARGET gm-demo-end-fit PROPERTY DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})

IF (gramods_INSTALL_DEMOS)
  INSTALL(TARGETS gm-demo-end-fit
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
ENDIF()

LIST(APPEND gramods_EXEC_TARGET_FILES "$<TARGET_FILE:gm-demo-end-fit>")
SET(gramods_EXEC_TARGET_FILES ${gramods_EXEC_TARGET_FILES} PARENT_SCOPE)
