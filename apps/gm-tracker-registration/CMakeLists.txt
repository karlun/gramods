CMAKE_MINIMUM_REQUIRED(VERSION 3.14)
CMAKE_POLICY(VERSION 3.14)


SET (APP_INCLUDE_DIRS CACHE INTERNAL "The list of include folders to search for the app.")
SET (APP_LIBS CACHE INTERNAL "The list of libraries that the app should link against.")

SET (APP_INCLUDE_DIRS ${APP_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/include)
SET (APP_INCLUDE_DIRS ${APP_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR}/include)

SET (APP_LIBS ${APP_LIBS} gmCore)
SET (APP_LIBS ${APP_LIBS} gmTrack)


FIND_PACKAGE (TCLAP)
IF (NOT TCLAP_FOUND)
  MESSAGE (STATUS "Not including gm-tracker-registration - requires TCLAP")
  RETURN()
ENDIF ()
SET (APP_INCLUDE_DIRS ${APP_INCLUDE_DIRS} ${TCLAP_INCLUDE_DIRS})

FIND_PACKAGE(Threads)
IF (NOT Threads_FOUND)
  MESSAGE (STATUS "Not including gm-tracker-registration - requires Threads")
  RETURN()
ENDIF ()
SET (APP_LIBS ${APP_LIBS} ${CMAKE_THREAD_LIBS_INIT})


ADD_EXECUTABLE(gm-tracker-registration src/main.cpp)

TARGET_INCLUDE_DIRECTORIES(gm-tracker-registration PUBLIC ${APP_INCLUDE_DIRS})
TARGET_LINK_LIBRARIES(gm-tracker-registration ${APP_LIBS})

SET_PROPERTY(TARGET gm-tracker-registration PROPERTY CXX_STANDARD 14)

INSTALL(TARGETS gm-tracker-registration
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

INCLUDE(InstallDependencies)
install_executable_dependencies($<TARGET_FILE:gm-tracker-registration>)
