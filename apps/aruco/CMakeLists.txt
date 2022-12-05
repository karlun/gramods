CMAKE_MINIMUM_REQUIRED(VERSION 3.10)
CMAKE_POLICY(VERSION 3.10)


IF (NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/src)
  MESSAGE(STATUS "To include aruco examples, copy opencv_contrib/modules/aruco/samples to gramods/apps/aruco/src or use soft link.")
  RETURN()
ENDIF()


SET (APP_INCLUDE_DIRS CACHE INTERNAL "The list of include folders to search for the app.")
SET (APP_LIBS CACHE INTERNAL "The list of libraries that the app should link against.")

LIST (APPEND APP_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/src)
LIST (APPEND APP_INCLUDE_DIRS ${CMAKE_CURRENT_BINARY_DIR}/src)


FIND_PACKAGE(OpenCV QUIET OPTIONAL_COMPONENTS videoio highgui aruco)
IF(OpenCV_FOUND AND NOT OPENCV_ARUCO_FOUND)
  MESSAGE(STATUS "Not including aruco samples - OpenCV without aruco module")
  RETURN()
ENDIF()
IF(NOT OPENCV_ARUCO_FOUND)
  MESSAGE(STATUS "Not including aruco samples - required OpenCV with aruco module")
  RETURN()
ENDIF()
IF (NOT OpenCV_VERSION_MAJOR EQUAL 4)
  MESSAGE(WARNING "OpenCV version is not 4 - errors may occur")
ENDIF()
LIST (APPEND APP_LIBS ${OpenCV_LIBS})

OPTION(gramods_INSTALL_aruco "Install the aruco apps" OFF)

FILE(GLOB aruco_sample_files ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
FOREACH (FILENAME ${aruco_sample_files})

  GET_FILENAME_COMPONENT(APP ${FILENAME} NAME_WE)

  ADD_EXECUTABLE(aruco_${APP} ${FILENAME})

  TARGET_INCLUDE_DIRECTORIES(aruco_${APP} PUBLIC ${APP_INCLUDE_DIRS})
  TARGET_LINK_LIBRARIES(aruco_${APP} ${APP_LIBS})

  SET_PROPERTY(TARGET aruco_${APP} PROPERTY CXX_STANDARD 17)
  SET_PROPERTY(TARGET aruco_${APP} PROPERTY DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})

  IF(gramods_INSTALL_aruco)
    INSTALL(TARGETS aruco_${APP}
      LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
      RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
    LIST(APPEND gramods_EXEC_TARGET_FILES "$<TARGET_FILE:aruco_${APP}>")
  ENDIF()

ENDFOREACH()

SET(gramods_EXEC_TARGET_FILES ${gramods_EXEC_TARGET_FILES} PARENT_SCOPE)
