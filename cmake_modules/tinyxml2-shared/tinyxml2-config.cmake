# Final resort if no "real" tinyxml2-config.cmake file is available

if(NOT TinyXML2_ROOT_DIR AND DEFINED ENV{TinyXML2_ROOT_DIR})
  SET(TinyXML2_ROOT_DIR "$ENV{TinyXML2_ROOT_DIR}" CACHE PATH
    "TinyXML2 base directory location (optional, used for nonstandard installation paths)")
endif()

if(TinyXML2_ROOT_DIR)
  SET(TinyXML2_INCLUDE_PATH PATHS "${TinyXML2_ROOT_DIR}/include" NO_DEFAULT_PATH)
  SET(TinyXML2_LIBRARY_PATH PATHS "${TinyXML2_ROOT_DIR}/lib"     NO_DEFAULT_PATH)
endif()

FIND_PATH(TinyXML2_INCLUDE_DIR NAMES tinyxml2.h PATH_SUFFIXES "tinyxml2" ${TinyXML2_INCLUDE_PATH})
FIND_LIBRARY(TinyXML2_LIBRARY  NAMES tinyxml2   PATH_SUFFIXES "tinyxml2" ${TinyXML2_LIBRARY_PATH})

ADD_LIBRARY(tinyxml2::tinyxml2 SHARED IMPORTED)
IF(TinyXML2_INCLUDE_DIR AND TinyXML2_LIBRARY)
  SET_TARGET_PROPERTIES(tinyxml2::tinyxml2 PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${TinyXML2_INCLUDE_DIR}
    IMPORTED_LOCATION ${TinyXML2_LIBRARY}
    INTERFACE_LINK_LIBRARIES tinyxml2)
ELSE()
  MESSAGE(SEND_ERROR "Could not set properties of target tinyxml2::tinyxml2 - set TinyXML2_INCLUDE_DIR and TinyXML2_LIBRARY or unset TinyXML2_DIR")
ENDIF()
