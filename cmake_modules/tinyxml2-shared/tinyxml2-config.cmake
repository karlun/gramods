# Final resort if no "real" tinyxml2-config.cmake file is available

IF (NOT WIN32)

  FIND_PATH(TinyXML2_INCLUDE_DIR NAMES tinyxml2.h
    DOC "Path to TinyXML2 include folder")
  FIND_LIBRARY(TinyXML2_LIBRARY NAMES tinyxml2
    DOC "Path to shared library for TinyXML2")

  ADD_LIBRARY(tinyxml2::tinyxml2 SHARED IMPORTED)
  IF(TinyXML2_INCLUDE_DIR AND TinyXML2_LIBRARY)
    SET_TARGET_PROPERTIES(tinyxml2::tinyxml2 PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES ${TinyXML2_INCLUDE_DIR}
      IMPORTED_LOCATION ${TinyXML2_LIBRARY})
  ELSE()
    MESSAGE(SEND_ERROR "Could not set properties of target tinyxml2::tinyxml2 - set TinyXML2_* or unset TinyXML2_DIR")
  ENDIF()

ELSE()

  FIND_PATH(TinyXML2_INCLUDE_DIR NAMES tinyxml2.h PATH_SUFFIXES "tinyxml2"
    DOC "Path to TinyXML2 include folder")
  FIND_FILE(TinyXML2_LIBRARY NAMES tinyxml2.dll PATH_SUFFIXES "bin"
    DOC "Path to shared library for TinyXML2 (DLL on Windows)")
  FIND_LIBRARY(TinyXML2_IMPLIB NAMES tinyxml2 PATH_SUFFIXES "lib"
    DOC "Path to library file for TinyXML2 (.lib file)")

  ADD_LIBRARY(tinyxml2::tinyxml2 SHARED IMPORTED)
  IF(TinyXML2_INCLUDE_DIR AND TinyXML2_LIBRARY AND TinyXML2_IMPLIB)
    SET_TARGET_PROPERTIES(tinyxml2::tinyxml2 PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES ${TinyXML2_INCLUDE_DIR}
      IMPORTED_LOCATION ${TinyXML2_LIBRARY}
      IMPORTED_IMPLIB ${TinyXML2_IMPLIB})
  ELSE()
    MESSAGE(SEND_ERROR "Could not set properties of target tinyxml2::tinyxml2 - set TinyXML2_* or unset TinyXML2_DIR")
  ENDIF()

ENDIF()
