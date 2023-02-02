CMAKE_MINIMUM_REQUIRED(VERSION 3.13)
CMAKE_POLICY(VERSION 3.13)


SET (PRIVATE_INCLUDE_DIRS CACHE INTERNAL "Includes *not* used in module header files")
SET (PUBLIC_LIBS CACHE INTERNAL "Libraries with headers used in module header files")
SET (PRIVATE_LIBS CACHE INTERNAL "Libraries *not* used in module header files.")
SET (INTERNAL_LIBS CACHE INTERNAL "Internal targets and dependencies with full paths.")


FIND_PACKAGE (Eigen3 QUIET NO_MODULE)
IF (Eigen3_FOUND)
  SET(HAVE_Eigen3 1)
  OPTION(gramods_ENABLE_Eigen3 "Enable functionality that requires Eigen3" ON)
  IF (gramods_ENABLE_Eigen3)
    LIST (APPEND PUBLIC_LIBS Eigen3::Eigen)
  ENDIF()
ENDIF()

FIND_PACKAGE(TinyXML2 CONFIG)
IF(TinyXML2_FOUND)
  SET(HAVE_TinyXML2 1)
  OPTION(gramods_ENABLE_TinyXML2 "Enable functionality that requires TinyXML2" ON)
  IF(gramods_ENABLE_TinyXML2)
    LIST (APPEND PUBLIC_LIBS tinyxml2::tinyxml2)
  ELSE()
    MESSAGE (STATUS "gmCore will not support XML-based configuration - requires TinyXML2 (disabled)")
  ENDIF()
ELSE()
    MESSAGE (WARNING "gmCore will not support XML-based configuration - requires TinyXML2")
ENDIF()

FIND_PACKAGE(SDL2 CONFIG)
IF(SDL2_FOUND)
  SET(HAVE_SDL2 1)
  OPTION(gramods_ENABLE_SDL2 "Enable functionality that requires SDL2" ON)
  IF(gramods_ENABLE_SDL2)
    LIST (APPEND PUBLIC_LIBS SDL2::SDL2)
  ELSE()
    MESSAGE (STATUS "gmCore will not be able to initialize SDL2 context - requires SDL2 (disabled)")
  ENDIF()
ELSE()
    MESSAGE (WARNING "gmCore will not be able to initialize SDL2 context - requires SDL2")
ENDIF()

FIND_PACKAGE(FreeImage CONFIG)
IF(FreeImage_FOUND)
  SET(HAVE_FreeImage 1)
  OPTION(gramods_ENABLE_FreeImage "Enable functionality that requires FreeImage" ON)
  IF (gramods_ENABLE_FreeImage)
    LIST (APPEND PUBLIC_LIBS freeimage::FreeImage)
  ELSE()
    MESSAGE (STATUS "gmCore will not support image loading - requires FreeImage (disabled)")
  ENDIF()
ELSE()
  MESSAGE (WARNING "gmCore will not support image loading - requires FreeImage")
ENDIF()

FIND_PACKAGE(OpenVR CONFIG)
IF(OpenVR_FOUND)
  SET(HAVE_OpenVR 1)
  OPTION(gramods_ENABLE_OpenVR "Enable functionality that requires OpenVR" ON)
  IF (gramods_ENABLE_OpenVR)
    LIST (APPEND PUBLIC_LIBS OpenVR::OpenVR)
  ENDIF(gramods_ENABLE_OpenVR)
ELSE()
  MESSAGE (WARNING "Not including OpenVR support in gmCore - requires OpenVR")
ENDIF()

LIST (APPEND PRIVATE_LIBS ${CMAKE_DL_LIBS})


OPTION(gramods_STRIP_PATH_FROM_FILE
  "Strip full path from source file in debug output through Console (GM_ERR, GM_WRN, etc)"
  OFF)

CONFIGURE_FILE(
  ${CMAKE_CURRENT_SOURCE_DIR}/src/config_cmake.hh
  ${CMAKE_CURRENT_BINARY_DIR}/include/gmCore/config.hh)


INCLUDE(GetSourcesAndHeaders)
get_sources_and_headers()

LIST(APPEND HEADERS_MAGIC $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include/gmCore/config.hh>)
LIST(APPEND HEADERS_MAGIC $<INSTALL_INTERFACE:include/gmCore/config.hh>)
LIST(APPEND HEADERS_MAGIC $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include/gmCore/export.hh>)
LIST(APPEND HEADERS_MAGIC $<INSTALL_INTERFACE:include/gmCore/export.hh>)

SOURCE_GROUP("headers" FILES ${HEADERS_ABSOLUTE})
SOURCE_GROUP("sources" FILES ${SOURCES})

ADD_LIBRARY(gmCore SHARED)

TARGET_INCLUDE_DIRECTORIES(gmCore
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
  $<INSTALL_INTERFACE:include>)
TARGET_SOURCES(gmCore
  PRIVATE ${SOURCES}
  PUBLIC ${HEADERS_MAGIC})
TARGET_LINK_LIBRARIES(gmCore
  PUBLIC ${PUBLIC_LIBS}
  PRIVATE ${PRIVATE_LIBS})

# Get absolute library paths of dependencies into apps and tests
ADD_LIBRARY(gmCore_internal_deps INTERFACE)
TARGET_LINK_LIBRARIES(gmCore_internal_deps
  INTERFACE ${PUBLIC_LIBS} ${PRIVATE_LIBS} ${INTERNAL_LIBS} gmCore)


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

LIST(APPEND gramods_LIB_TARGET_FILES "$<TARGET_FILE:gmCore>")
SET(gramods_LIB_TARGET_FILES ${gramods_LIB_TARGET_FILES} PARENT_SCOPE)
