CMAKE_MINIMUM_REQUIRED(VERSION 3.14)
CMAKE_POLICY(VERSION 3.14)


SET (MODULE_INCLUDE_DIRS CACHE INTERNAL "The list of include folders to search for the core module.")
SET (MODULE_LIBS CACHE INTERNAL "The list of libraries that the core module should link against.")

LIST (APPEND MODULE_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include)
LIST (APPEND MODULE_INCLUDE_DIRS ${CMAKE_CURRENT_BINARY_DIR}/include)

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
    LIST (APPEND MODULE_INCLUDE_DIRS ${TinyXML2_INCLUDE_DIRS})
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
    LIST (APPEND MODULE_INCLUDE_DIRS ${SDL2_INCLUDE_DIR})
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
  ${CMAKE_CURRENT_SOURCE_DIR}/include/gmCore/config_cmake.hh
  ${CMAKE_CURRENT_BINARY_DIR}/include/gmCore/config.hh)


FILE(GLOB_RECURSE HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hh)
FILE(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

SOURCE_GROUP("headers" FILES ${HEADERS})
SOURCE_GROUP("sources" FILES ${SOURCES})

ADD_LIBRARY(gmCore SHARED ${HEADERS} ${SOURCES})

TARGET_INCLUDE_DIRECTORIES(gmCore PUBLIC ${MODULE_INCLUDE_DIRS})
TARGET_LINK_LIBRARIES(gmCore ${MODULE_LIBS})

INCLUDE(GenerateExportHeader)
GENERATE_EXPORT_HEADER(gmCore
  EXPORT_FILE_NAME include/gmCore/export.hh
  EXPORT_MACRO_NAME gmCore_API)

SET_PROPERTY(TARGET gmCore PROPERTY CXX_STANDARD 17)
SET_PROPERTY(TARGET gmCore PROPERTY WINDOWS_EXPORT_ALL_SYMBOLS TRUE)

INSTALL(TARGETS gmCore
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
INSTALL(DIRECTORY include/
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

INCLUDE(InstallDependencies)
install_library_dependencies($<TARGET_FILE:gmCore>)
