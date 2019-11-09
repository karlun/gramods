CMAKE_MINIMUM_REQUIRED(VERSION 3.10.2)
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

SUBDIRLIST(gramods_MODULES ${gramods_SOURCE_DIR}/modules)
FOREACH (MODULE ${gramods_MODULES})

  OPTION(gramods_INCLUDE_${MODULE} "Include the module ${MODULE}" ON)

  IF (NOT gramods_INCLUDE_${MODULE})
    CONTINUE()
  ENDIF()

  IF (NOT EXISTS ${gramods_SOURCE_DIR}/modules/${MODULE}/CMakeLists.txt)
    MESSAGE (STATUS "Skipping ${MODULE} - not a module")
    CONTINUE()
  ENDIF ()

  MESSAGE (STATUS "Adding module ${MODULE}" )
  ADD_SUBDIRECTORY (${gramods_SOURCE_DIR}/modules/${MODULE})

ENDFOREACH ()

# --- HANDLE APPS ---

SUBDIRLIST(gramods_APPS ${gramods_SOURCE_DIR}/apps)
FOREACH (APP ${gramods_APPS})

  OPTION(gramods_INCLUDE_${APP} "Include the app ${APP}" ON)

  IF (NOT gramods_INCLUDE_${APP})
    CONTINUE()
  ENDIF()

  IF (NOT EXISTS ${gramods_SOURCE_DIR}/apps/${APP}/CMakeLists.txt)
    MESSAGE (STATUS "Skipping ${APP} - not an app")
    CONTINUE()
  ENDIF ()

  MESSAGE (STATUS "Adding app ${APP}" )
  ADD_SUBDIRECTORY (${gramods_SOURCE_DIR}/apps/${APP})

ENDFOREACH ()

# --- PACKING ---

INCLUDE(InstallRequiredSystemLibraries)

SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "The gramods package is a collection of modules for graphics rendering.")
SET(CPACK_PACKAGE_NAME "libgramods")
SET(CPACK_PACKAGE_VENDOR "Linköping University")
SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/DESCRIPTION.txt")
SET(CPACK_PACKAGE_CONTACT "Karljohan Lundin Palmerius <karljohan.lundin.palmerius@liu.se>")

INCLUDE(CPack)

INSTALL(DIRECTORY config/
  DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/config)

# --- TESTING ---

FIND_PACKAGE(GTest)
IF (GTest_FOUND)
  ENABLE_TESTING()

  SUBDIRLIST(SUBDIRS ${gramods_SOURCE_DIR}/test)
  
  FOREACH (TEST ${SUBDIRS})
    IF (EXISTS ${gramods_SOURCE_DIR}/test/${TEST}/CMakeLists.txt)
      MESSAGE (STATUS "Adding test suite ${TEST}" )
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
  FIND_PACKAGE(Python3)

  OPTION(gramods_DOCS_IN_ALL "Include building documentation in the ALL target." ON)
  IF (gramods_DOCS_IN_ALL)
    SET(gramods_DOCS_ALL "ALL")
  ELSE()
    SET(gramods_DOCS_ALL "")
  ENDIF()

  CONFIGURE_FILE (
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules/Doxyfile.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)

  IF (Python3_FOUND)
    ADD_CUSTOM_TARGET (docs ${gramods_DOCS_ALL}
      COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/xml-attributes
      COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/generate_xml_attribute_documentation.py --modules ${CMAKE_CURRENT_SOURCE_DIR}/modules --output ${CMAKE_CURRENT_BINARY_DIR}/xml-attributes
      COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      COMMENT "Generating API documentation with Doxygen" VERBATIM
      )
  ELSE()
    MESSAGE (WARNING "Cannot generate documentation over XML tags and attributes without Python 3 - executable python3 could not be found.")
    ADD_CUSTOM_TARGET (docs ${gramods_DOCS_ALL}
      COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      COMMENT "Generating API documentation with Doxygen" VERBATIM
      )
  ENDIF()

  INSTALL(
    DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/doc_doxygen/html/
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/docs)

ELSE ()
  MESSAGE ("Include Doxygen to enable building documentation")
ENDIF (DOXYGEN_FOUND)

