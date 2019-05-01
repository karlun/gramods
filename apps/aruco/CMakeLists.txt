CMAKE_MINIMUM_REQUIRED(VERSION 2.8.3)


IF (NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/src)
  MESSAGE(STATUS "To include aruco examples, copy .../aruco/samples to ${CMAKE_CURRENT_SOURCE_DIR}/src or use soft link.")
  RETURN()
ENDIF()


SET (APP_INCLUDE_DIRS CACHE INTERNAL "The list of include folders to search for the app.")
SET (APP_LIBS CACHE INTERNAL "The list of libraries that the app should link against.")

SET (APP_INCLUDE_DIRS ${APP_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/src)
SET (APP_INCLUDE_DIRS ${APP_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR}/src)


FIND_PACKAGE(OpenCV QUIET OPTIONAL_COMPONENTS aruco)
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
SET (APP_LIBS ${APP_LIBS} ${OpenCV_LIBS})

FOREACH (APP
    calibrate_camera_charuco
    calibrate_camera
    create_board_charuco
    create_board
    create_diamond
    create_marker
    detect_board_charuco
    detect_board
    detect_diamonds
    detect_markers)

  ADD_EXECUTABLE(aruco_${APP} "src/${APP}.cpp")

  TARGET_INCLUDE_DIRECTORIES(aruco_${APP} PUBLIC ${APP_INCLUDE_DIRS})
  TARGET_LINK_LIBRARIES(aruco_${APP} ${APP_LIBS})

  SET_PROPERTY(TARGET aruco_${APP} PROPERTY CXX_STANDARD 14)

  INSTALL(TARGETS aruco_${APP}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

ENDFOREACH()
