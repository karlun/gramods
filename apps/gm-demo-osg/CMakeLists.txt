CMAKE_MINIMUM_REQUIRED(VERSION 3.10)
CMAKE_POLICY(VERSION 3.10)


SET (APP_INCLUDE_DIRS CACHE INTERNAL "The list of include folders to search for the app.")
SET (APP_LIBS CACHE INTERNAL "The list of libraries that the app should link against.")

SET(gramods_PARTS
  ${gramods_PARTS} app_gm_demo_osg PARENT_SCOPE)

LIST (APPEND APP_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include)
LIST (APPEND APP_INCLUDE_DIRS ${CMAKE_CURRENT_BINARY_DIR}/include)


IF (NOT TARGET gmCore)
  SET(gramods_REQ_app_gm_demo_osg "gmCore" PARENT_SCOPE)
  RETURN()
ENDIF ()
LIST (APPEND APP_LIBS gmCore_internal_deps)

IF (NOT TARGET gmGraphics)
  SET(gramods_REQ_app_gm_demo_osg "gmGraphics" PARENT_SCOPE)
  RETURN()
ENDIF ()
LIST (APPEND APP_LIBS gmGraphics_internal_deps)

IF (NOT TARGET gmNetwork)
  SET(gramods_REQ_app_gm_demo_osg "gmNetwork" PARENT_SCOPE)
  RETURN()
ENDIF ()
LIST (APPEND APP_LIBS gmNetwork_internal_deps)

IF (NOT TARGET gmTrack)
  SET(gramods_REQ_app_gm_demo_osg "gmTrack" PARENT_SCOPE)
  RETURN()
ENDIF ()
LIST (APPEND APP_LIBS gmTrack_internal_deps)

IF (NOT gramods_ENABLE_TinyXML2)
  SET(gramods_REQ_app_gm_demo_osg "XML" PARENT_SCOPE)
  RETURN()
ENDIF ()

FIND_PACKAGE(OpenSceneGraph COMPONENTS osgGA osgViewer osgDB QUIET)
IF (NOT OPENSCENEGRAPH_FOUND)
  SET(gramods_REQ_app_gm_demo_osg "OpenSceneGraph" PARENT_SCOPE)
  RETURN()
ENDIF()
SET(HAVE_OpenSceneGraph 1)
OPTION(gramods_ENABLE_OpenSceneGraph "Enable functionality that requires OpenSceneGraph" ON)
 IF (NOT gramods_ENABLE_OpenSceneGraph)
   SET(gramods_REQ_app_gm_demo_osg "OpenSceneGraph" PARENT_SCOPE)
   RETURN()
ENDIF()
LIST (APPEND APP_INCLUDE_DIRS ${OPENSCENEGRAPH_INCLUDE_DIRS})
LIST (APPEND APP_LIBS ${OPENSCENEGRAPH_LIBRARIES})


FIND_PACKAGE(Threads QUIET)
IF (NOT Threads_FOUND)
  SET(gramods_REQ_app_gm_demo_osg "Threads" PARENT_SCOPE)
  RETURN()
ENDIF ()
LIST (APPEND APP_LIBS Threads::Threads)


ADD_EXECUTABLE(gm-demo-osg src/main.cpp src/MyApp.cpp src/MyApp.hh)

TARGET_INCLUDE_DIRECTORIES(gm-demo-osg PUBLIC ${APP_INCLUDE_DIRS})
TARGET_LINK_LIBRARIES(gm-demo-osg ${APP_LIBS})

SET_PROPERTY(TARGET gm-demo-osg PROPERTY CXX_STANDARD 17)
SET_PROPERTY(TARGET gm-demo-osg PROPERTY DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})

IF (gramods_INSTALL_DEMOS)
  INSTALL(TARGETS gm-demo-osg
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
  INSTALL(DIRECTORY config/
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/gramods/config)
  INSTALL(DIRECTORY resources/
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/gramods/resources)
ENDIF()

LIST(APPEND gramods_EXEC_TARGET_FILES "$<TARGET_FILE:gm-demo-osg>")
SET(gramods_EXEC_TARGET_FILES ${gramods_EXEC_TARGET_FILES} PARENT_SCOPE)
