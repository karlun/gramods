CMAKE_MINIMUM_REQUIRED(VERSION 3.10)
CMAKE_POLICY(VERSION 3.10)

PROJECT(gramods)

LIST (APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules")
INCLUDE(GNUInstallDirs)

# Platform fixes
# Definition to force MSVC into standard complient aligned storage
ADD_DEFINITIONS(-D_ENABLE_EXTENDED_ALIGNED_STORAGE)

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

# --- GENERAL OPTIONS ---

OPTION(gramods_INSTALL_RESOURCES "Include module resources in INSTALL target." ON)

OPTION(gramods_INSTALL_DEPENDENCIES "Attempt to install third party libraries together with libraries and executables." OFF)

OPTION(gramods_BUILD_RESOURCES "Include building resources in the ALL target." ON)
IF (gramods_BUILD_RESOURCES)
  SET(gramods_RESOURCE_ALL "ALL" CACHE INTERNAL "The keyword 'ALL' for all resource targets")
ELSE()
  SET(gramods_RESOURCE_ALL "" CACHE INTERNAL "The keyword 'ALL' omitted for all resource targets")
ENDIF()

# --- HANDLE MODULES ---

SUBDIRLIST(gramods_MODULES ${gramods_SOURCE_DIR}/modules)
FOREACH (MODULE ${gramods_MODULES})

  OPTION(gramods_MODULE_${MODULE} "Include the module ${MODULE}" ON)

  IF (NOT gramods_MODULE_${MODULE})
    CONTINUE()
  ENDIF()

  IF (NOT EXISTS ${gramods_SOURCE_DIR}/modules/${MODULE}/CMakeLists.txt)
    MESSAGE (STATUS "Could not find build configuration modules/${MODULE}/CMakeLists.txt")
    CONTINUE()
  ENDIF ()

  MESSAGE (STATUS "Adding module ${MODULE}" )
  ADD_SUBDIRECTORY (${gramods_SOURCE_DIR}/modules/${MODULE})

ENDFOREACH ()

# --- HANDLE APPS ---

SUBDIRLIST(gramods_APPS ${gramods_SOURCE_DIR}/apps)
FOREACH (APP ${gramods_APPS})

  OPTION(gramods_APP_${APP} "Include the app ${APP}" ON)

  IF (NOT gramods_APP_${APP})
    CONTINUE()
  ENDIF()

  IF (NOT EXISTS ${gramods_SOURCE_DIR}/apps/${APP}/CMakeLists.txt)
    MESSAGE (STATUS "Could not find build configuration apps/${APP}/CMakeLists.txt")
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
SET(CPACK_PACKAGE_VERSION_MAJOR 0)
SET(CPACK_PACKAGE_VERSION_MINOR 1)
SET(CPACK_PACKAGE_VERSION_PATCH 1)

INCLUDE(CPack)

INSTALL(DIRECTORY config/
  DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/gramods/config)
INSTALL(FILES cmake_modules/gramods.urn
  DESTINATION bin)

# --- TESTING ---

FIND_PACKAGE(GTest)
IF (GTest_FOUND)
  ENABLE_TESTING()

  SUBDIRLIST(SUBDIRS ${gramods_SOURCE_DIR}/test)
  
  FOREACH (TEST ${SUBDIRS})

    OPTION(gramods_TEST_${TEST} "Include test ${TEST}" ON)

    IF (NOT gramods_TEST_${TEST})
      CONTINUE()
    ENDIF()

    IF (NOT EXISTS ${gramods_SOURCE_DIR}/test/${TEST}/CMakeLists.txt)
      MESSAGE (WARNING "Could not find build configuration test/${TEST}/CMakeLists.txt")
      CONTINUE()
    ENDIF ()

    MESSAGE (STATUS "Adding test suite ${TEST}" )
    ADD_SUBDIRECTORY (${gramods_SOURCE_DIR}/test/${TEST})

  ENDFOREACH ()
ELSE ()
  MESSAGE ("Include gTest to enable building test suites")
ENDIF ()

# --- DOCUMENTATION ---

FIND_PACKAGE(Doxygen)
IF (DOXYGEN_FOUND)

  FIND_PACKAGE(PlantUML)
  FIND_PACKAGE(Python3)

  OPTION(gramods_BUILD_DOCUMENTATION "Include building documentation in the ALL target." ON)
  IF (gramods_BUILD_DOCUMENTATION)
    SET(gramods_DOCS_ALL "ALL" CACHE INTERNAL "The keyword 'ALL' for all documentation targets")
  ELSE()
    SET(gramods_DOCS_ALL "" CACHE INTERNAL "The keyword 'ALL' omitted for all documentation targets")
  ENDIF()

  IF (PlantUML_JARFILE)
    SET(gramods_PlantUML_JARFILE "${PlantUML_JARFILE}")
  ELSE()
    SET(gramods_PlantUML_JARFILE "")
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

  OPTION(gramods_INSTALL_DOCUMENTATION "Install documentation." ON)
  IF (gramods_INSTALL_DOCUMENTATION)
    INSTALL(
      DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/doc_doxygen/html/
      DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/gramods/docs)
  ENDIF()

ELSE ()
  MESSAGE ("Include Doxygen to enable building documentation")
ENDIF (DOXYGEN_FOUND)

