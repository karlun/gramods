# Find the TCLAP includes and library
#
#  TCLAP_INCLUDE_DIRS   - where to find TCLAP include files
#  TCLAP_FOUND          - True if TCLAP was found

FIND_PATH(TCLAP_INCLUDE_DIRS tclap/ValueArg.h
  /usr/include /usr/local/include
)

# handle the QUIETLY and REQUIRED arguments and set TCLAP_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(TCLAP DEFAULT_MSG TCLAP_INCLUDE_DIRS)
