CMAKE_MINIMUM_REQUIRED(VERSION 3.13)
CMAKE_POLICY(VERSION 3.13)


SET (PUBLIC_INCLUDE_DIRS CACHE INTERNAL "The list of public include folders for the graphics module.")
SET (PRIVATE_INCLUDE_DIRS CACHE INTERNAL "The list of private include folders for the graphics module.")
SET (MODULE_LIBS CACHE INTERNAL "The list of libraries that the graphics module must link against.")


LIST (APPEND MODULE_LIBS gmCore)
LIST (APPEND MODULE_LIBS gmTrack)

FIND_PACKAGE (Eigen3 QUIET NO_MODULE)
IF (Eigen3_FOUND)
  SET(HAVE_Eigen3 1)
  OPTION(gramods_ENABLE_Eigen3 "Enable functionality that requires Eigen3" ON)
  IF (NOT gramods_ENABLE_Eigen3)
    MESSAGE (STATUS "Not including gmGraphics - requires Eigen3 (disabled)")
    RETURN()
  ENDIF()
ELSE()
  MESSAGE (WARNING "Not including gmGraphics - requires Eigen3")
  RETURN()
ENDIF()
LIST (APPEND MODULE_LIBS Eigen3::Eigen)

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
    MESSAGE (WARNING "Not including gmGraphics - requires GLEW")
  ENDIF()
  IF (NOT OPENGL_FOUND)
    MESSAGE (WARNING "Not including gmGraphics - requires OpenGL")
  ENDIF()
  RETURN()
ENDIF()
LIST (APPEND MODULE_LIBS GLEW::GLEW)
LIST (APPEND MODULE_LIBS OpenGL::GL)


FIND_PACKAGE(SDL2)
IF(SDL2_FOUND)
  SET(HAVE_SDL2 1)
  OPTION(gramods_ENABLE_SDL2 "Enable functionality that requires SDL2" ON)
  IF(gramods_ENABLE_SDL2)
    LIST (APPEND PRIVATE_INCLUDE_DIRS ${SDL2_INCLUDE_DIR})
    LIST (APPEND MODULE_LIBS ${SDL2_LIBRARY})
  ENDIF(gramods_ENABLE_SDL2)
ELSE()
  MESSAGE (WARNING "Not including SDL2 window support in gmGraphics - requires SDL2")
ENDIF(SDL2_FOUND)

FIND_PACKAGE (libuvc)
IF(libuvc_FOUND)
  SET(HAVE_libuvc 1)
  OPTION(gramods_ENABLE_libuvc "Enable functionality that requires libuvc" ON)
  IF (gramods_ENABLE_libuvc)
    LIST (APPEND PRIVATE_INCLUDE_DIRS ${libuvc_INCLUDE_DIR})
    LIST (APPEND MODULE_LIBS ${libuvc_LIBRARIES})
  ENDIF(gramods_ENABLE_libuvc)
ENDIF(libuvc_FOUND)

FIND_PACKAGE(FreeImage)
IF(FreeImage_FOUND)
  SET(HAVE_FreeImage 1)
  OPTION(gramods_ENABLE_FreeImage "Enable functionality that requires FreeImage" ON)
  IF (gramods_ENABLE_FreeImage)
    LIST (APPEND PRIVATE_INCLUDE_DIRS ${FreeImage_INCLUDE_DIR})
    LIST (APPEND MODULE_LIBS ${FreeImage_LIBRARIES})
  ENDIF(gramods_ENABLE_FreeImage)
ELSE()
  MESSAGE (WARNING "Not including image loading support in gmGraphics - requires FreeImage")
ENDIF(FreeImage_FOUND)

FIND_PACKAGE(OpenSceneGraph COMPONENTS osgViewer)
IF (OPENSCENEGRAPH_FOUND)
  SET(HAVE_OpenSceneGraph 1)
  OPTION(gramods_ENABLE_OpenSceneGraph "Enable functionality that requires OpenSceneGraph" ON)
  IF (gramods_ENABLE_OpenSceneGraph)
    LIST (APPEND PRIVATE_INCLUDE_DIRS ${OPENSCENEGRAPH_INCLUDE_DIRS})
    LIST (APPEND MODULE_LIBS ${OPENSCENEGRAPH_LIBRARIES})
  ELSE()
    MESSAGE (STATUS "Not including OpenSceneGraph support in gmGraphics - requires OpenSceneGraph (disabled)")
  ENDIF()
ELSE()
  MESSAGE (STATUS "Not including OpenSceneGraph support in gmGraphics - requires OpenSceneGraph")
ENDIF()

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
    ${gramods_RESOURCE_ALL}
    DEPENDS ${SVGPNG_RESOURCE_TARGETS})

  MESSAGE (STATUS "Added 'svg-resources' target for converting SVG resources to PNG.")
ELSE()
  MESSAGE (STATUS "Not including 'svg-resources' target - requires Inkscape")
ENDIF()

INCLUDE(PythonResources)
IF(PythonResources_FOUND)

  SET (PYPNG_RESOURCE_TARGETS CACHE INTERNAL "The list of png resources that can be built with python.")

  FILE(GLOB_RECURSE PY_RESOURCES ${CMAKE_CURRENT_SOURCE_DIR}/resources/*.py)
  SOURCE_GROUP("python-resources" FILES ${PY_RESOURCES})

  SET(gramods_PNG_RESOURCE_SIZE 1024 CACHE STRING "The width of generated image resources, in pixels.")

  FOREACH (RESOURCE ${PY_RESOURCES})
    STRING(REGEX REPLACE
      "${CMAKE_CURRENT_SOURCE_DIR}/resources/(.*)\\.py\$"
      "${CMAKE_CURRENT_BINARY_DIR}/resources/\\1.png"
      PNG_RESOURCE ${RESOURCE})
    MESSAGE("${RESOURCE} -> ${PNG_RESOURCE}")
    python_build_image(${RESOURCE} ${PNG_RESOURCE} ${gramods_PNG_RESOURCE_SIZE})
    LIST (APPEND PYPNG_RESOURCE_TARGETS ${PNG_RESOURCE})
  ENDFOREACH()

  add_custom_target(python-resources
    ${gramods_RESOURCE_ALL}
    DEPENDS ${PYPNG_RESOURCE_TARGETS})

  MESSAGE (STATUS "Added 'python-resources' target for creating PNG with python.")
ELSE()
  MESSAGE (STATUS "Not including 'python-resources' target - requires Python3")
ENDIF()

IF (gramods_INSTALL_RESOURCES AND EXISTS ${CMAKE_CURRENT_BINARY_DIR}/resources)
  INSTALL(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/resources
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/gramods/gmGraphics)
ENDIF()

CONFIGURE_FILE(
  ${CMAKE_CURRENT_SOURCE_DIR}/src/config_cmake.hh
  ${CMAKE_CURRENT_BINARY_DIR}/include/gmGraphics/config.hh)


INCLUDE(GetSourcesAndHeaders)
get_sources_and_headers()

LIST(APPEND HEADERS_MAGIC $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include/gmGraphics/config.hh>)
LIST(APPEND HEADERS_MAGIC $<INSTALL_INTERFACE:include/gmGraphics/config.hh>)

SOURCE_GROUP("headers" FILES ${HEADERS_ABSOLUTE})
SOURCE_GROUP("sources" FILES ${SOURCES})

ADD_LIBRARY(gmGraphics SHARED)

TARGET_INCLUDE_DIRECTORIES(gmGraphics
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
  $<INSTALL_INTERFACE:include>
  ${PUBLIC_INCLUDE_DIRS}
  PRIVATE
  ${PRIVATE_INCLUDE_DIRS})
TARGET_SOURCES(gmGraphics
  PRIVATE ${SOURCES}
  PUBLIC ${HEADERS_MAGIC}
  )
TARGET_LINK_LIBRARIES(gmGraphics ${MODULE_LIBS})

SET_PROPERTY(TARGET gmGraphics PROPERTY CXX_STANDARD 17)
SET_PROPERTY(TARGET gmGraphics PROPERTY WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
SET_PROPERTY(TARGET gmGraphics PROPERTY PUBLIC_HEADER ${HEADERS_MAGIC})

INSTALL(TARGETS gmGraphics
  EXPORT Gramods-gmGraphics-targets
  ARCHIVE LIBRARY RUNTIME
  PUBLIC_HEADER DESTINATION include/gmGraphics)
INSTALL(EXPORT Gramods-gmGraphics-targets
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/gramods/cmake)

INSTALL(DIRECTORY config/
  DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/gramods/gmGraphics/config)

SET(gramods_LIB_TARGET_FILES ${gramods_LIB_TARGET_FILES} "$<TARGET_FILE:gmGraphics>" PARENT_SCOPE)
