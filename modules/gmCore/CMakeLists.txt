CMAKE_MINIMUM_REQUIRED(VERSION 2.8.3)


SET (MODULE_INCLUDE_DIRS CACHE INTERNAL "The list of include folders to search for the core module.")
SET (MODULE_LIBS CACHE INTERNAL "The list of libraries that the core module should link against.")

SET (MODULE_INCLUDE_DIRS ${MODULE_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/include)
SET (MODULE_INCLUDE_DIRS ${MODULE_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR}/include)


FIND_PACKAGE(TinyXML2 REQUIRED)
SET (MODULE_INCLUDE_DIRS ${MODULE_INCLUDE_DIRS} ${TinyXML2_INCLUDE_DIRS})
SET (MODULE_LIBS ${MODULE_LIBS} ${TinyXML2_LIBRARIES})


OPTION(gramods_STRIP_PATH_FROM_FILE
  "Strip full path from source file in debug output through Console (GM_ERR, GM_WRN, etc)"
  OFF)

CONFIGURE_FILE(
  ${CMAKE_CURRENT_SOURCE_DIR}/include/gmCore/config_cmake.hh
  ${CMAKE_CURRENT_BINARY_DIR}/include/gmCore/config.hh)


ADD_LIBRARY(gmCore SHARED
  src/CommandLineParser.cpp
  src/Configuration.cpp
  src/Console.cpp
  src/ImportLibrary.cpp
  src/MessageSink.cpp
  src/OFactory.cpp
  src/OStreamMessageSink.cpp
  )

TARGET_INCLUDE_DIRECTORIES(gmCore PUBLIC ${MODULE_INCLUDE_DIRS})
TARGET_LINK_LIBRARIES(gmCore ${MODULE_LIBS})

SET_PROPERTY(TARGET gmCore PROPERTY CXX_STANDARD 14)
