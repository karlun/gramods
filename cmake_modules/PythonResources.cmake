#
# This module adds function
# python_build_image(scriptname, filename.png, size)
#

FIND_PROGRAM(PipEnv pipenv)

function (python_build_image script filename size)

  IF (NOT PipEnv)
    MESSAGE(ERROR "Could not call ${script} - pipenv missing!")
    RETURN()
  ENDIF()

  ADD_CUSTOM_COMMAND(
    OUTPUT Pipfile.lock
    COMMAND pwd
    COMMAND ${PipEnv} --bare install
    COMMENT "Installing pipenv dependencies [${PipEnv} install]"
    DEPENDS Pipfile
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )

  ADD_CUSTOM_COMMAND(
    OUTPUT ${filename}
    DEPENDS ${script} Pipfile.lock
    COMMAND ${PipEnv} run python ${script} -r ${size} -o ${filename} VERBATIM
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )

endfunction()

INCLUDE(FindPackageHandleStandardArgs)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(PythonResources REQUIRED_VARS PipEnv)
