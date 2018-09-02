CMAKE_MINIMUM_REQUIRED(VERSION 2.8.3)
PROJECT(gramods)

SET (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules")
INCLUDE(GNUInstallDirs)

# Find folders in a folder
MACRO(SUBDIRLIST result curdir)
  FILE(GLOB children RELATIVE ${curdir} ${curdir}/*)
  SET(dirlist "")
  FOREACH(child ${children})
    IF(IS_DIRECTORY ${curdir}/${child})
      LIST(APPEND dirlist ${child})
    ENDIF()
  ENDFOREACH()
  SET(${result} ${dirlist})
ENDMACRO()

# --- HANDLE MODULES ---

IF (NOT DEFINED gramods_MODULES)
  SUBDIRLIST(SUBDIRS ${gramods_SOURCE_DIR}/modules)
  SET (gramods_MODULES ${SUBDIRS} CACHE STRING "Add names of modules that should be included. Separate each name by ;. The modules' CMakeLists.txt must be located in ${gramods_SOURCE_DIR}/modules/(name of toolkit)" FORCE)
  MESSAGE (STATUS "Automatically creating modules list: " ${gramods_MODULES})
ENDIF ()

FOREACH (MODULE ${gramods_MODULES})
  IF (EXISTS ${gramods_SOURCE_DIR}/modules/${MODULE}/CMakeLists.txt)
    MESSAGE (STATUS "Including module ${MODULE}" )
    ADD_SUBDIRECTORY (${gramods_SOURCE_DIR}/modules/${MODULE})
  ELSE ()
    MESSAGE (WARNING "Unable to include module ${MODULE}")
  ENDIF ()
ENDFOREACH ()

# --- PACKING ---

INCLUDE(InstallRequiredSystemLibraries)

SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "The gramods package is a collection of modules for graphics rendering.")
SET(CPACK_PACKAGE_NAME "libgramods")
SET(CPACK_PACKAGE_VENDOR "Linköping University")
SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
SET(CPACK_PACKAGE_CONTACT "Karljohan Lundin Palmerius <karljohan.lundin.palmerius@liu.se>")

INCLUDE(CPack)

# --- TESTING ---

FIND_PACKAGE(GTest)
IF (GTest_FOUND)
  ENABLE_TESTING()

  SUBDIRLIST(SUBDIRS ${gramods_SOURCE_DIR}/test)
  
  FOREACH (TEST ${SUBDIRS})
    IF (EXISTS ${gramods_SOURCE_DIR}/test/${TEST}/CMakeLists.txt)
      MESSAGE (STATUS "Including test suite ${TEST}" )
      ADD_SUBDIRECTORY (${gramods_SOURCE_DIR}/test/${TEST})
    ELSE ()
      MESSAGE (WARNING "Unable to include test suite ${TEST} --- missing file test/${TEST}/CMakeLists.txt")
    ENDIF ()
  ENDFOREACH ()
ELSE ()
  MESSAGE ("Include gTest to enable building test suites")
ENDIF ()

# --- DOCUMENTATION ---

FIND_PACKAGE(Doxygen)
IF (DOXYGEN_FOUND)
  FIND_PACKAGE(PlantUML)
  CONFIGURE_FILE (
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules/Doxyfile.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
  ADD_CUSTOM_TARGET (doc
    ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating API documentation with Doxygen" VERBATIM
    )
ELSE ()
  MESSAGE ("Include Doxygen to enable building documentation")
ENDIF (DOXYGEN_FOUND)

