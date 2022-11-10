CMAKE_MINIMUM_REQUIRED(VERSION 3.10)
CMAKE_POLICY(VERSION 3.10)


SET (APP_INCLUDE_DIRS CACHE INTERNAL "The list of include folders to search for the app.")
SET (APP_LIBS CACHE INTERNAL "The list of libraries that the app should link against.")

LIST (APPEND APP_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include)
LIST (APPEND APP_INCLUDE_DIRS ${CMAKE_CURRENT_BINARY_DIR}/include)


IF (NOT TARGET gmCore)
  MESSAGE (STATUS "Not including gm-demo-osg - requires gmCore")
  RETURN()
ENDIF ()
LIST (APPEND APP_LIBS gmCore_internal_deps)

IF (NOT TARGET gmGraphics)
  MESSAGE (STATUS "Not including gm-demo-osg - requires gmGraphics")
  RETURN()
ENDIF ()
LIST (APPEND APP_LIBS gmGraphics_internal_deps)

IF (NOT TARGET gmNetwork)
  MESSAGE (STATUS "Not including gm-demo-osg - requires gmNetwork")
  RETURN()
ENDIF ()
LIST (APPEND APP_LIBS gmNetwork)

IF (NOT TARGET gmTrack)
  MESSAGE (STATUS "Not including gm-demo-osg - requires gmTrack")
  RETURN()
ENDIF ()
LIST (APPEND APP_LIBS gmTrack_internal_deps)

IF (NOT gramods_ENABLE_TinyXML2)
  MESSAGE (STATUS "Not including gm-demo-osg - requires XML support")
  RETURN()
ENDIF ()

FIND_PACKAGE(OpenSceneGraph COMPONENTS osgGA osgViewer osgDB)
IF (NOT OPENSCENEGRAPH_FOUND)
  MESSAGE (STATUS "Not including gm-demo-osg - requires OpenSceneGraph")
  RETURN()
ENDIF()
SET(HAVE_OpenSceneGraph 1)
OPTION(gramods_ENABLE_OpenSceneGraph "Enable functionality that requires OpenSceneGraph" ON)
IF (NOT gramods_ENABLE_OpenSceneGraph)
  MESSAGE (STATUS "Not including gm-demo-osg - requires OpenSceneGraph (disabled)")
  RETURN()
ENDIF()
LIST (APPEND APP_INCLUDE_DIRS ${OPENSCENEGRAPH_INCLUDE_DIRS})
LIST (APPEND APP_LIBS ${OPENSCENEGRAPH_LIBRARIES})

FIND_PACKAGE(Threads)
IF (NOT Threads_FOUND)
  MESSAGE (STATUS "Not including gm-demo-osg - requires Threads")
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
