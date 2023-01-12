# Final resort if no "real" freeimage-config.cmake file is available

if(NOT FreeImage_ROOT_DIR AND DEFINED ENV{FreeImage_ROOT_DIR})
  SET(FreeImage_ROOT_DIR "$ENV{FreeImage_ROOT_DIR}" CACHE PATH
    "FreeImage base directory location (optional, used for nonstandard installation paths)")
endif()

if(FreeImage_ROOT_DIR)
  SET(FreeImage_INCLUDE_PATH PATHS "${FreeImage_ROOT_DIR}/include" NO_DEFAULT_PATH)
  SET(FreeImage_LIBRARY_PATH PATHS "${FreeImage_ROOT_DIR}/lib"     NO_DEFAULT_PATH)
endif()

FIND_PATH(FreeImage_INCLUDE_DIR NAMES FreeImage.h PATH_SUFFIXES "FreeImage" ${FreeImage_INCLUDE_PATH})
FIND_LIBRARY(FreeImage_LIBRARY NAMES freeimage PATH_SUFFIXES "FreeImage" ${FreeImage_LIBRARY_PATH})

ADD_LIBRARY(freeimage::FreeImage SHARED IMPORTED)
IF(FreeImage_INCLUDE_DIR AND FreeImage_LIBRARY)
  SET_TARGET_PROPERTIES(freeimage::FreeImage PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${FreeImage_INCLUDE_DIR}
    IMPORTED_LOCATION ${FreeImage_LIBRARY}
    INTERFACE_LINK_LIBRARIES freeimage
    )
ELSE()
  MESSAGE(SEND_ERROR "Could not set properties of target freeimage::FreeImage - set FreeImage_INCLUDE_DIR and FreeImage_LIBRARY or unset FreeImage_DIR")
ENDIF()
