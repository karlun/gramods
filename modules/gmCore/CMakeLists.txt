CMAKE_MINIMUM_REQUIRED(VERSION 3.13)
CMAKE_POLICY(VERSION 3.13)


SET (PUBLIC_INCLUDE_DIRS CACHE INTERNAL "The list of public include folders for the core module.")
SET (MODULE_LIBS CACHE INTERNAL "The list of libraries that the core module must link against.")


FIND_PACKAGE (Eigen3 QUIET NO_MODULE)
IF (Eigen3_FOUND)
  SET(HAVE_Eigen3 1)
  OPTION(gramods_ENABLE_Eigen3 "Enable functionality that requires Eigen3" ON)
  IF (gramods_ENABLE_Eigen3)
    LIST (APPEND MODULE_LIBS Eigen3::Eigen)
  ENDIF()
ENDIF()

FIND_PACKAGE(TinyXML2)
IF(TinyXML2_FOUND)
  SET(HAVE_TinyXML2 1)
  OPTION(gramods_ENABLE_TinyXML2 "Enable functionality that requires TinyXML2" ON)
  IF(gramods_ENABLE_TinyXML2)
    LIST (APPEND PUBLIC_INCLUDE_DIRS ${TinyXML2_INCLUDE_DIRS})
    LIST (APPEND MODULE_LIBS ${TinyXML2_LIBRARIES})
    LIST (APPEND MODULE_DEFINES TINYXML2_IMPORT)
  ELSE()
    MESSAGE (STATUS "gmCore will not support XML-based configuration - requires TinyXML2 (disabled)")
  ENDIF()
ELSE()
    MESSAGE (WARNING "gmCore will not support XML-based configuration - requires TinyXML2")
ENDIF()

FIND_PACKAGE(SDL2)
IF(SDL2_FOUND)
  SET(HAVE_SDL2 1)
  OPTION(gramods_ENABLE_SDL2 "Enable functionality that requires SDL2" ON)
  IF(gramods_ENABLE_SDL2)
    LIST (APPEND PUBLIC_INCLUDE_DIRS ${SDL2_INCLUDE_DIR})
    LIST (APPEND MODULE_LIBS ${SDL2_LIBRARY})
  ELSE()
    MESSAGE (STATUS "gmCore will not be able to initialize SDL2 context - requires SDL2 (disabled)")
  ENDIF()
ELSE()
    MESSAGE (WARNING "gmCore will not be able to initialize SDL2 context - requires SDL2")
ENDIF()

LIST (APPEND MODULE_LIBS ${CMAKE_DL_LIBS})


OPTION(gramods_STRIP_PATH_FROM_FILE
  "Strip full path from source file in debug output through Console (GM_ERR, GM_WRN, etc)"
  OFF)

CONFIGURE_FILE(
  ${CMAKE_CURRENT_SOURCE_DIR}/src/config_cmake.hh
  ${CMAKE_CURRENT_BINARY_DIR}/include/gmCore/config.hh)


INCLUDE(GetSourcesAndHeaders)
get_sources_and_headers()

SOURCE_GROUP("headers" FILES ${HEADERS_ABSOLUTE})
SOURCE_GROUP("sources" FILES ${SOURCES})

ADD_LIBRARY(gmCore SHARED)

TARGET_INCLUDE_DIRECTORIES(gmCore
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
  $<INSTALL_INTERFACE:include>
  ${PUBLIC_INCLUDE_DIRS})
TARGET_SOURCES(gmCore
  PRIVATE ${SOURCES}
  PUBLIC ${HEADERS_MAGIC}
  )
TARGET_LINK_LIBRARIES(gmCore ${MODULE_LIBS})


INCLUDE(GenerateExportHeader)
GENERATE_EXPORT_HEADER(gmCore
  EXPORT_FILE_NAME include/gmCore/export.hh
  EXPORT_MACRO_NAME gmCore_API)

SET_PROPERTY(TARGET gmCore PROPERTY CXX_STANDARD 17)
SET_PROPERTY(TARGET gmCore PROPERTY WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
SET_PROPERTY(TARGET gmCore PROPERTY PUBLIC_HEADER ${HEADERS_MAGIC})

INSTALL(TARGETS gmCore
  EXPORT Gramods-gmCore-targets
  ARCHIVE LIBRARY RUNTIME
  PUBLIC_HEADER DESTINATION include/gmCore)
INSTALL(EXPORT Gramods-gmCore-targets
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/gramods/cmake)

INCLUDE(InstallDependencies)
install_library_dependencies($<TARGET_FILE:gmCore>)
