CMAKE_MINIMUM_REQUIRED(VERSION 3.10)
CMAKE_POLICY(VERSION 3.10)


SET (APP_INCLUDE_DIRS CACHE INTERNAL "The list of include folders to search for the app.")
SET (APP_LIBS CACHE INTERNAL "The list of libraries that the app should link against.")

SET(gramods_PARTS
  ${gramods_PARTS} app_gm_demo_polyfit PARENT_SCOPE)


IF (NOT TARGET gmCore)
  SET(gramods_REQ_app_gm_demo_polyfit "gmCore" PARENT_SCOPE)
  RETURN()
ENDIF ()
LIST (APPEND APP_LIBS gmCore_internal_deps)

IF (NOT TARGET gmMisc)
  SET(gramods_REQ_app_gm_demo_multimisc "gmMisc" PARENT_SCOPE)
  RETURN()
ENDIF ()
LIST (APPEND APP_LIBS gmMisc_internal_deps)


IF (NOT gramods_ENABLE_Eigen3)
  SET(gramods_REQ_app_gm_demo_polyfit "Eigen3" PARENT_SCOPE)
  RETURN()
ENDIF ()

IF (NOT gramods_ENABLE_SDL2)
  SET(gramods_REQ_app_gm_demo_polyfit "SDL2" PARENT_SCOPE)
  RETURN()
ENDIF ()


FILE(GLOB_RECURSE HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hh)
FILE(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

SOURCE_GROUP("headers" FILES ${HEADERS})
SOURCE_GROUP("sources" FILES ${SOURCES})

ADD_EXECUTABLE(gm-demo-polyfit ${HEADERS} ${SOURCES})

TARGET_INCLUDE_DIRECTORIES(gm-demo-polyfit PUBLIC ${APP_INCLUDE_DIRS})
TARGET_LINK_LIBRARIES(gm-demo-polyfit ${APP_LIBS})

SET_PROPERTY(TARGET gm-demo-polyfit PROPERTY CXX_STANDARD 20)
SET_PROPERTY(TARGET gm-demo-polyfit PROPERTY DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})

IF (gramods_INSTALL_DEMOS)
  INSTALL(TARGETS gm-demo-polyfit
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
ENDIF()

LIST(APPEND gramods_EXEC_TARGET_FILES "$<TARGET_FILE:gm-demo-polyfit>")
SET(gramods_EXEC_TARGET_FILES ${gramods_EXEC_TARGET_FILES} PARENT_SCOPE)
