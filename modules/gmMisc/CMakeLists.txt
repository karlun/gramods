CMAKE_MINIMUM_REQUIRED(VERSION 3.13)
CMAKE_POLICY(VERSION 3.13)


SET (PRIVATE_INCLUDE_DIRS CACHE INTERNAL "Includes *not* used in module header files")
SET (PUBLIC_LIBS CACHE INTERNAL "Libraries with headers used in module header files")
SET (PRIVATE_LIBS CACHE INTERNAL "Libraries *not* used in module header files.")
SET (INTERNAL_LIBS CACHE INTERNAL "Internal targets and dependencies with full paths.")


LIST (APPEND PUBLIC_LIBS gmCore)
LIST (APPEND INTERNAL_LIBS gmCore_internal_deps)

IF (NOT gramods_ENABLE_Eigen3)
  MESSAGE (WARNING "Many feature in gmMisc missing since they require Eigen3")
ENDIF()

FIND_PACKAGE(fftw)
IF (fftw_FOUND)
  SET (HAVE_fftw 1)
  LIST(APPEND gramods_DOCS_DEFINES HAVE_fftw)
  OPTION(gramods_ENABLE_fftw "Enable functionality that requires fftw" ON)
  IF (gramods_ENABLE_fftw)
    LIST (APPEND PUBLIC_LIBS fftw::fftw)
  ENDIF()
ENDIF ()

FIND_PATH (LEHDARI_DELAUNAY_INCLUDE "Lehdari/Delaunay.hpp"
  DOC "Path to include folder containing Lehdari Delaunay header.")
IF (LEHDARI_DELAUNAY_INCLUDE)
  SET (HAVE_Lehdari_Delaunay 1)
  LIST(APPEND gramods_DOCS_DEFINES HAVE_Lehdari_Delaunay)
  OPTION(gramods_ENABLE_Lehdari_Delaunay "Enable functionality that requires Lehdari Delaunay" ON)
  IF (gramods_ENABLE_Lehdari_Delaunay)
    ADD_LIBRARY(lehdari::delaunay INTERFACE IMPORTED)
    SET_TARGET_PROPERTIES(lehdari::delaunay PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES ${LEHDARI_DELAUNAY_INCLUDE})
    LIST (APPEND PUBLIC_LIBS lehdari::delaunay)
  ENDIF()
ENDIF()


SET(gramods_DOCS_DEFINES ${gramods_DOCS_DEFINES} PARENT_SCOPE)


CONFIGURE_FILE(
  ${CMAKE_CURRENT_SOURCE_DIR}/src/config_cmake.hh
  ${CMAKE_CURRENT_BINARY_DIR}/include/gmMisc/config.hh)


INCLUDE(GetSourcesAndHeaders)
get_sources_and_headers()

LIST(APPEND HEADERS_MAGIC $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include/gmMisc/config.hh>)
LIST(APPEND HEADERS_MAGIC $<INSTALL_INTERFACE:include/gmMisc/config.hh>)

SOURCE_GROUP("headers" FILES ${HEADERS_ABSOLUTE})
SOURCE_GROUP("sources" FILES ${SOURCES})

ADD_LIBRARY(gmMisc SHARED)

TARGET_INCLUDE_DIRECTORIES(gmMisc
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
  $<INSTALL_INTERFACE:include>
  PRIVATE
  ${PRIVATE_INCLUDE_DIRS})
TARGET_SOURCES(gmMisc
  PRIVATE ${SOURCES}
  PUBLIC ${HEADERS_MAGIC})
TARGET_LINK_LIBRARIES(gmMisc
  PUBLIC ${PUBLIC_LIBS}
  PRIVATE ${PRIVATE_LIBS})

# Get absolute library paths of dependencies into apps and tests
ADD_LIBRARY(gmMisc_internal_deps INTERFACE)
TARGET_LINK_LIBRARIES(gmMisc_internal_deps
  INTERFACE ${PUBLIC_LIBS} ${PRIVATE_LIBS} ${INTERNAL_LIBS} gmMisc)


SET_PROPERTY(TARGET gmMisc PROPERTY CXX_STANDARD 17)
SET_PROPERTY(TARGET gmMisc PROPERTY WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
SET_PROPERTY(TARGET gmMisc PROPERTY PUBLIC_HEADER ${HEADERS_MAGIC})

INSTALL(TARGETS gmMisc
  EXPORT Gramods-gmMisc-targets
  ARCHIVE LIBRARY RUNTIME
  PUBLIC_HEADER DESTINATION include/gmMisc)
INSTALL(EXPORT Gramods-gmMisc-targets
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/gramods/cmake)

LIST(APPEND gramods_LIB_TARGET_FILES "$<TARGET_FILE:gmMisc>")
SET(gramods_LIB_TARGET_FILES ${gramods_LIB_TARGET_FILES} PARENT_SCOPE)
