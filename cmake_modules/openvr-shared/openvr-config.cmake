# Final resort if no "real" openvr-config.cmake file is available

IF (NOT WIN32)

  FIND_PATH(OpenVR_INCLUDE_DIR NAMES openvr.h PATH_SUFFIXES "openvr" DOC "Path to openvr headers folder")
  FIND_LIBRARY(OpenVR_LIBRARY NAMES openvr DOC "Path to openvr shared library file")

  ADD_LIBRARY(OpenVR::OpenVR SHARED IMPORTED)
  IF(OpenVR_INCLUDE_DIR AND OpenVR_LIBRARY)
    SET_TARGET_PROPERTIES(OpenVR::OpenVR PROPERTIES
	    INTERFACE_INCLUDE_DIRECTORIES ${OpenVR_INCLUDE_DIR}
	    IMPORTED_LOCATION ${OpenVR_LIBRARY}
	    )
  ELSE()
    MESSAGE(SEND_ERROR "Could not set properties of target OpenVR::OpenVR - set OpenVR_INCLUDE_DIR and OpenVR_LIBRARY or unset OpenVR_DIR")
  ENDIF()

ELSE()

  FIND_PATH(OpenVR_INCLUDE_DIR NAMES openvr.h DOC "Path to openvr headers folder")
  FIND_FILE(OpenVR_LIBRARY NAMES openvr_api.dll PATH_SUFFIXES "bin" DOC "Path to openvr dll file")
  FIND_LIBRARY(OpenVR_IMPLIB NAMES openvr PATH_SUFFIXES "lib" DOC "Path to openvr lib file")

  ADD_LIBRARY(OpenVR::OpenVR SHARED IMPORTED)
  IF(OpenVR_INCLUDE_DIR AND OpenVR_LIBRARY AND OpenVR_IMPLIB)
    SET_TARGET_PROPERTIES(OpenVR::OpenVR PROPERTIES
	    INTERFACE_INCLUDE_DIRECTORIES ${OpenVR_INCLUDE_DIR}
	    IMPORTED_LOCATION ${OpenVR_LIBRARY}
	    IMPORTED_IMPLIB ${OpenVR_IMPLIB}
	    )
  ELSE()
    MESSAGE(SEND_ERROR "Could not set properties of target OpenVR::OpenVR - set all OpenVR_* paths or unset OpenVR_DIR")
  ENDIF()

ENDIF()

