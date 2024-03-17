# Final resort if no "real" fftw-config.cmake file is available

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
