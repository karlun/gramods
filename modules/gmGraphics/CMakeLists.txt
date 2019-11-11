CMAKE_MINIMUM_REQUIRED(VERSION 2.8.3)


SET (MODULE_INCLUDE_DIRS CACHE INTERNAL "The list of include folders to search for the graphics module.")
SET (MODULE_LIBS CACHE INTERNAL "The list of libraries that the graphics module should link against.")

SET (MODULE_INCLUDE_DIRS ${MODULE_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/include)
SET (MODULE_INCLUDE_DIRS ${MODULE_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR}/include)


SET (MODULE_LIBS ${MODULE_LIBS} gmCore)
SET (MODULE_LIBS ${MODULE_LIBS} gmTypes)
SET (MODULE_LIBS ${MODULE_LIBS} gmTrack)

FIND_PACKAGE (Eigen3 QUIET NO_MODULE)
IF (Eigen3_FOUND)
  SET(HAVE_Eigen3 1)
  OPTION(gramods_ENABLE_Eigen3 "Enable functionality that requires Eigen3" ON)
  IF (NOT gramods_ENABLE_Eigen3)
    MESSAGE (STATUS "Not including gmGraphics - requires Eigen3 (disabled)")
    RETURN()
  ENDIF()
ELSE()
  MESSAGE (STATUS "Not including gmGraphics - requires Eigen3")
  RETURN()
ENDIF()
SET (MODULE_LIBS ${MODULE_LIBS} Eigen3::Eigen)

IF (POLICY CMP0072)
  CMAKE_POLICY(SET CMP0072 NEW)
ENDIF (POLICY CMP0072)

FIND_PACKAGE(GLEW)
FIND_PACKAGE(OpenGL)
IF (OPENGL_FOUND AND GLEW_FOUND)
  OPTION(gramods_ENABLE_OpenGL "Enable functionality that requires OpenGL" ON)
  IF (NOT gramods_ENABLE_OpenGL)
    MESSAGE (STATUS "Not including gmGraphics - requires OpenGL (disabled)")
    RETURN()
  ENDIF()
ELSE()
  IF (NOT GLEW_FOUND)
    MESSAGE (STATUS "Not including gmGraphics - requires GLEW")
  ENDIF()
  IF (NOT OPENGL_FOUND)
    MESSAGE (STATUS "Not including gmGraphics - requires OpenGL")
  ENDIF()
  RETURN()
ENDIF()
SET (MODULE_LIBS ${MODULE_LIBS} GLEW::GLEW)
SET (MODULE_LIBS ${MODULE_LIBS} OpenGL::GL)


FIND_PACKAGE(SDL2)
IF(SDL2_FOUND)
  SET(HAVE_SDL2 1)
  OPTION(gramods_ENABLE_SDL2 "Enable functionality that requires SDL2" ON)
  IF(gramods_ENABLE_SDL2)
    SET (MODULE_INCLUDE_DIRS ${MODULE_INCLUDE_DIRS} ${SDL2_INCLUDE_DIR})
    SET (MODULE_LIBS ${MODULE_LIBS} ${SDL2_LIBRARY})
  ENDIF(gramods_ENABLE_SDL2)
ENDIF(SDL2_FOUND)

FIND_PACKAGE (libuvc)
IF(libuvc_FOUND)
  SET(HAVE_libuvc 1)
  OPTION(gramods_ENABLE_libuvc "Enable functionality that requires libuvc" ON)
  IF (gramods_ENABLE_libuvc)
    SET (MODULE_INCLUDE_DIRS ${MODULE_INCLUDE_DIRS} ${libuvc_INCLUDE_DIR})
    SET (MODULE_LIBS ${MODULE_LIBS} ${libuvc_LIBRARIES})
  ENDIF(gramods_ENABLE_libuvc)
ENDIF(libuvc_FOUND)

FIND_PACKAGE(FreeImage)
IF(FreeImage_FOUND)
  SET(HAVE_FreeImage 1)
  OPTION(gramods_ENABLE_FreeImage "Enable functionality that requires FreeImage" ON)
  IF (gramods_ENABLE_FreeImage)
    SET (MODULE_INCLUDE_DIRS ${MODULE_INCLUDE_DIRS} ${FreeImage_INCLUDE_DIR})
    SET (MODULE_LIBS ${MODULE_LIBS} ${FreeImage_LIBRARIES})
  ENDIF(gramods_ENABLE_FreeImage)
ENDIF(FreeImage_FOUND)

FIND_PACKAGE(Inkscape)
IF(Inkscape_FOUND)

  SET (SVGPNG_RESOURCE_TARGETS CACHE INTERNAL "The list of png resources that can be built from svg.")

  FILE(GLOB_RECURSE SVG_RESOURCES ${CMAKE_CURRENT_SOURCE_DIR}/resources/*.svg)
  SOURCE_GROUP("svg-resources" FILES ${SVG_RESOURCES})

  FOREACH (RESOURCE ${SVG_RESOURCES})
    STRING(REGEX REPLACE
      "${CMAKE_CURRENT_SOURCE_DIR}/(.*)\\.svg$"
      "${CMAKE_CURRENT_BINARY_DIR}/\\1.png"
      SVGPNG_RESOURCE ${RESOURCE})
    convert_svg_to_png(${RESOURCE} ${SVGPNG_RESOURCE})
    LIST (APPEND SVGPNG_RESOURCE_TARGETS ${SVGPNG_RESOURCE})
  ENDFOREACH()

  add_custom_target(svg-resources
    ${RESOURCE_ALL}
    DEPENDS ${SVGPNG_RESOURCE_TARGETS})
  INSTALL(FILES ${SVGPNG_RESOURCE_TARGETS}
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/gmGraphics/resources)

  MESSAGE (STATUS "Added 'svg-resources' target for converting SVG resources to PNG.")
ELSE()
  MESSAGE (STATUS "Not including 'svg-resources' target - requires Inkscape")
ENDIF()

FIND_PACKAGE(GimpConsole)
IF(GimpConsole_FOUND)

  SET (PYPNG_RESOURCE_TARGETS CACHE INTERNAL "The list of png resources that can be built with gimp python-fu.")

  FILE(GLOB_RECURSE PY_RESOURCES ${CMAKE_CURRENT_SOURCE_DIR}/resources/*.py)
  SOURCE_GROUP("python-fu-resources" FILES ${PY_RESOURCES})

  FOREACH (RESOURCE ${PY_RESOURCES})
    STRING(REGEX REPLACE
      "${CMAKE_CURRENT_SOURCE_DIR}/(.*)\\.py$"
      "${CMAKE_CURRENT_BINARY_DIR}/\\1.png"
      PNG_RESOURCE ${RESOURCE})
    gimp_build_image(${RESOURCE} ${PNG_RESOURCE})
    LIST (APPEND PYPNG_RESOURCE_TARGETS ${PNG_RESOURCE})
  ENDFOREACH()

  add_custom_target(python-fu-resources
    ${RESOURCE_ALL}
    DEPENDS ${PYPNG_RESOURCE_TARGETS})
  INSTALL(FILES ${PYPNG_RESOURCE_TARGETS}
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/gmGraphics/resources)

  MESSAGE (STATUS "Added 'python-fu-resources' target for creating PNG with gimp python-fu.")
ELSE()
  MESSAGE (STATUS "Not including 'python-fu-resources' target - requires gimp-console")
ENDIF()


CONFIGURE_FILE(
  ${CMAKE_CURRENT_SOURCE_DIR}/include/gmGraphics/config_cmake.hh
  ${CMAKE_CURRENT_BINARY_DIR}/include/gmGraphics/config.hh)


FILE(GLOB_RECURSE HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hh)
FILE(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

SOURCE_GROUP("headers" FILES ${HEADERS})
SOURCE_GROUP("sources" FILES ${SOURCES})

ADD_LIBRARY(gmGraphics SHARED ${HEADERS} ${SOURCES})

TARGET_INCLUDE_DIRECTORIES(gmGraphics PUBLIC ${MODULE_INCLUDE_DIRS})
TARGET_LINK_LIBRARIES(gmGraphics ${MODULE_LIBS})

SET_PROPERTY(TARGET gmGraphics PROPERTY CXX_STANDARD 14)
SET_PROPERTY(TARGET gmGraphics PROPERTY WINDOWS_EXPORT_ALL_SYMBOLS TRUE)

INSTALL(TARGETS gmGraphics
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
INSTALL(DIRECTORY include/
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
INSTALL(DIRECTORY config/
  DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/gmGraphics/config)
