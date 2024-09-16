# Final resort if no "real" fftw-config.cmake file is available

IF (NOT WIN32)

  FIND_PATH(fftw_INCLUDE_DIR NAMES fftw3.h
    DOC "Path to fftw include folder")
  FIND_LIBRARY(fftw_LIBRARY NAMES fftw3
    DOC "Path to shared library for fftw")

  ADD_LIBRARY(fftw::fftw SHARED IMPORTED)
  IF(fftw_INCLUDE_DIR AND fftw_LIBRARY)
    SET_TARGET_PROPERTIES(fftw::fftw PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES ${fftw_INCLUDE_DIR}
      IMPORTED_LOCATION ${fftw_LIBRARY})
  ELSE()
    MESSAGE(SEND_ERROR "Could not set properties of target fftw::fftw - set fftw_* or unset fftw_DIR")
  ENDIF()

ELSE()

  FIND_PATH(fftw_INCLUDE_DIR NAMES fftw3.h
    DOC "Path to fftw include folder")
  FIND_FILE(fftw_LIBRARY NAMES fftw3.dll PATH_SUFFIXES "bin"
    DOC "Path to shared library for fftw (DLL on Windows)")
  FIND_LIBRARY(fftw_IMPLIB NAMES fftw3.lib PATH_SUFFIXES "lib"
    DOC "Path to library file for fftw (.lib file)")

  ADD_LIBRARY(fftw::fftw SHARED IMPORTED)
  IF(fftw_INCLUDE_DIR AND fftw_LIBRARY AND fftw_IMPLIB)
    SET_TARGET_PROPERTIES(fftw::fftw PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES ${fftw_INCLUDE_DIR}
      IMPORTED_LOCATION ${fftw_LIBRARY}
      IMPORTED_IMPLIB ${fftw_IMPLIB})
  ELSE()
    MESSAGE(SEND_ERROR "Could not set properties of target fftw::fftw - set fftw_* or unset fftw_DIR")
  ENDIF()

ENDIF()
