CMAKE_MINIMUM_REQUIRED(VERSION 2.8.3)


SET (APP_INCLUDE_DIRS CACHE INTERNAL "The list of include folders to search for the app.")
SET (APP_LIBS CACHE INTERNAL "The list of libraries that the app should link against.")

LIST (APPEND APP_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include)
LIST (APPEND APP_INCLUDE_DIRS ${CMAKE_CURRENT_BINARY_DIR}/include)


IF (NOT TARGET gmCore)
  MESSAGE (WARNING "Not including gm-load - requires gmCore")
  RETURN()
ENDIF ()
LIST (APPEND APP_LIBS gmCore)

IF (NOT gramods_ENABLE_TinyXML2)
  MESSAGE (WARNING "Not including gm-load - requires XML support")
  RETURN()
ENDIF ()

IF (NOT TARGET gmGraphics)
  MESSAGE (WARNING "Not including gm-load - requires gmGraphics")
  RETURN()
ENDIF ()
LIST (APPEND APP_LIBS gmGraphics)

IF (NOT TARGET gmNetwork)
  MESSAGE (STATUS "Not including gm-load - requires gmNetwork")
  RETURN()
ENDIF ()
SET (APP_LIBS ${APP_LIBS} gmNetwork)


FIND_PACKAGE (TCLAP)
IF (NOT TCLAP_FOUND)
  MESSAGE (WARNING "Not including gm-load - requires TCLAP")
  RETURN()
ENDIF ()
LIST (APPEND APP_INCLUDE_DIRS ${TCLAP_INCLUDE_DIRS})

FIND_PACKAGE(Threads)
IF (NOT Threads_FOUND)
  MESSAGE (WARNING "Not including gm-load - requires Threads")
  RETURN()
ENDIF ()
LIST (APPEND APP_LIBS Threads::Threads)


ADD_EXECUTABLE(gm-load src/main.cpp)

TARGET_INCLUDE_DIRECTORIES(gm-load PUBLIC ${APP_INCLUDE_DIRS})
TARGET_LINK_LIBRARIES(gm-load ${APP_LIBS})

SET_PROPERTY(TARGET gm-load PROPERTY CXX_STANDARD 14)

INSTALL(TARGETS gm-load
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
INSTALL(DIRECTORY config/
  DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/config)
