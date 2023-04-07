# Final resort if no "real" freeimage-config.cmake file is available

IF (NOT WIN32)

  FIND_PATH(FreeImage_INCLUDE_DIR
    NAMES FreeImage.h
    DOC "Path to FreeImage include folder")
  FIND_LIBRARY(FreeImage_LIBRARY
    NAMES freeimage
    DOC "Path to shared library for FreeImage")

  ADD_LIBRARY(freeimage::FreeImage SHARED IMPORTED)
  IF(FreeImage_INCLUDE_DIR AND FreeImage_LIBRARY)
    SET_TARGET_PROPERTIES(freeimage::FreeImage PROPERTIES
	    INTERFACE_INCLUDE_DIRECTORIES ${FreeImage_INCLUDE_DIR}
	    IMPORTED_LOCATION ${FreeImage_LIBRARY}
	    )
  ELSE()
    MESSAGE(SEND_ERROR "Could not set properties of target freeimage::FreeImage - set FreeImage_* or unset FreeImage_DIR")
  ENDIF()

ELSE()

  FIND_PATH(FreeImage_INCLUDE_DIR
    NAMES FreeImage.h PATH_SUFFIXES "FreeImage"
    DOC "Path to FreeImage include folder")
  FIND_FILE(FreeImage_LIBRARY
    NAMES freeimage.dll PATH_SUFFIXES "bin"
    DOC "Path to shared library for FreeImage (DLL on Windows)")
  FIND_LIBRARY(FreeImage_IMPLIB
    NAMES freeimage PATH_SUFFIXES "lib"
    DOC "Path to library file for FreeImage (.lib file)")

  ADD_LIBRARY(freeimage::FreeImage SHARED IMPORTED)
  IF(FreeImage_INCLUDE_DIR AND FreeImage_LIBRARY AND FreeImage_IMPLIB)
    SET_TARGET_PROPERTIES(freeimage::FreeImage PROPERTIES
	    INTERFACE_INCLUDE_DIRECTORIES ${FreeImage_INCLUDE_DIR}
	    IMPORTED_LOCATION ${FreeImage_LIBRARY}
	    IMPORTED_IMPLIB ${FreeImage_IMPLIB}
	    )
  ELSE()
    MESSAGE(SEND_ERROR "Could not set properties of target freeimage::FreeImage - set FreeImage_* or unset FreeImage_DIR")
  ENDIF()

ENDIF()

