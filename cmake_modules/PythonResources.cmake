#
# This module adds function
# python_build_image(scriptname, filename.png, size)
#

FIND_PACKAGE(Python3)

function (python_build_image script filename size)
  
  IF (NOT Python3_FOUND)
    MESSAGE(ERROR "Could not call image resource generator script - Python missing!")
    RETURN()
  ENDIF()

  ADD_CUSTOM_COMMAND(
    OUTPUT ${filename}
    DEPENDS ${script}
    COMMAND ${Python3_EXECUTABLE} ${script} -r ${size} -o ${filename} VERBATIM
    )
endfunction()

INCLUDE(FindPackageHandleStandardArgs)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(PythonResources REQUIRED_VARS Python3_EXECUTABLE)
