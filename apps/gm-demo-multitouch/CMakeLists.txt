CMAKE_MINIMUM_REQUIRED(VERSION 3.10)
CMAKE_POLICY(VERSION 3.10)


SET (APP_INCLUDE_DIRS CACHE INTERNAL "The list of include folders to search for the app.")
SET (APP_LIBS CACHE INTERNAL "The list of libraries that the app should link against.")

SET(gramods_PARTS
  ${gramods_PARTS} app_gm_demo_multitouch PARENT_SCOPE)


IF (NOT TARGET gmCore)
  SET(gramods_REQ_app_gm_demo_multitouch "gmCore" PARENT_SCOPE)
  RETURN()
ENDIF ()
LIST (APPEND APP_LIBS gmCore_internal_deps)

IF (NOT TARGET gmTouch)
  SET(gramods_REQ_app_gm_demo_multitouch "gmTouch" PARENT_SCOPE)
  RETURN()
ENDIF ()
LIST (APPEND APP_LIBS gmTouch_internal_deps)


IF (NOT gramods_ENABLE_SDL2)
  SET(gramods_REQ_app_gm_demo_multitouch "SDL2" PARENT_SCOPE)
  RETURN()
ENDIF ()

FIND_PACKAGE(SDL2_ttf QUIET)
IF (SDL2_ttf_FOUND)
  SET(HAVE_SDL2_ttf 1)
  OPTION(gramods_ENABLE_SDL2_ttf "Enable functionality that requires SDL2 ttf" ON)
  IF(NOT gramods_ENABLE_SDL2)
    SET(gramods_REQ_app_gm_demo_multitouch "SDL2 ttf" PARENT_SCOPE)
    RETURN()
  ENDIF()
ELSE ()
  SET(gramods_REQ_app_gm_demo_multitouch "SDL2 ttf" PARENT_SCOPE)
  RETURN()
ENDIF ()
LIST (APPEND APP_INCLUDE_DIRS ${SDL2_ttf_INCLUDE_DIRS})
LIST (APPEND APP_LIBS ${SDL2_ttf_LIBRARIES})

FILE(GLOB_RECURSE HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hh)
FILE(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

SOURCE_GROUP("headers" FILES ${HEADERS})
SOURCE_GROUP("sources" FILES ${SOURCES})

ADD_EXECUTABLE(gm-demo-multitouch ${HEADERS} ${SOURCES})

TARGET_INCLUDE_DIRECTORIES(gm-demo-multitouch PUBLIC ${APP_INCLUDE_DIRS})
TARGET_LINK_LIBRARIES(gm-demo-multitouch ${APP_LIBS})

SET_PROPERTY(TARGET gm-demo-multitouch PROPERTY CXX_STANDARD 20)
SET_PROPERTY(TARGET gm-demo-multitouch PROPERTY DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})

IF (gramods_INSTALL_DEMOS)
  INSTALL(TARGETS gm-demo-multitouch
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
ENDIF()

LIST(APPEND gramods_EXEC_TARGET_FILES "$<TARGET_FILE:gm-demo-multitouch>")
SET(gramods_EXEC_TARGET_FILES ${gramods_EXEC_TARGET_FILES} PARENT_SCOPE)
