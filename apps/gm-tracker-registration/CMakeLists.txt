CMAKE_MINIMUM_REQUIRED(VERSION 3.10)
CMAKE_POLICY(VERSION 3.10)


SET (APP_INCLUDE_DIRS CACHE INTERNAL "The list of include folders to search for the app.")
SET (APP_LIBS CACHE INTERNAL "The list of libraries that the app should link against.")

LIST (APPEND APP_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include)
LIST (APPEND APP_INCLUDE_DIRS ${CMAKE_CURRENT_BINARY_DIR}/include)

IF (NOT TARGET gmCore)
  MESSAGE (WARNING "Not including gm-tracker-registration - requires gmCore")
  RETURN()
ENDIF ()
LIST (APPEND APP_LIBS gmCore_internal_deps)

IF (NOT gramods_ENABLE_TinyXML2)
  MESSAGE (WARNING "Not including gm-tracker-registration - requires XML support")
  RETURN()
ENDIF ()

IF (NOT TARGET gmTrack)
  MESSAGE (WARNING "Not including gm-tracker-registration - requires gmTrack")
  RETURN()
ENDIF ()
LIST (APPEND APP_LIBS gmTrack_internal_deps)


FIND_PACKAGE (TCLAP)
IF (NOT TCLAP_FOUND)
  MESSAGE (STATUS "Not including gm-tracker-registration - requires TCLAP")
  RETURN()
ENDIF ()
LIST (APPEND APP_INCLUDE_DIRS ${TCLAP_INCLUDE_DIRS})

FIND_PACKAGE(Threads)
IF (NOT Threads_FOUND)
  MESSAGE (STATUS "Not including gm-tracker-registration - requires Threads")
  RETURN()
ENDIF ()
LIST (APPEND APP_LIBS ${CMAKE_THREAD_LIBS_INIT})


ADD_EXECUTABLE(gm-tracker-registration src/main.cpp)

TARGET_INCLUDE_DIRECTORIES(gm-tracker-registration PUBLIC ${APP_INCLUDE_DIRS})
TARGET_LINK_LIBRARIES(gm-tracker-registration ${APP_LIBS})

SET_PROPERTY(TARGET gm-tracker-registration PROPERTY CXX_STANDARD 17)
SET_PROPERTY(TARGET gm-tracker-registration PROPERTY DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})

INSTALL(TARGETS gm-tracker-registration
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

LIST(APPEND gramods_EXEC_TARGET_FILES "$<TARGET_FILE:gm-tracker-registration>")
SET(gramods_EXEC_TARGET_FILES ${gramods_EXEC_TARGET_FILES} PARENT_SCOPE)
