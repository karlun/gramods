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

  cmake_path(GET script PARENT_PATH SCRIPT_FOLDER)

  ADD_CUSTOM_COMMAND(
    OUTPUT ${SCRIPT_FOLDER}/Pipfile.lock
    COMMAND pwd
    COMMAND ${PipEnv} --bare install
    COMMENT "Installing pipenv dependencies in ${SCRIPT_FOLDER} [${PipEnv} install]"
    DEPENDS ${SCRIPT_FOLDER}/Pipfile
    WORKING_DIRECTORY ${SCRIPT_FOLDER}
  )

  cmake_path(GET script FILENAME SCRIPT_FILE)

  ADD_CUSTOM_COMMAND(
    OUTPUT ${filename}
    DEPENDS ${script} ${SCRIPT_FOLDER}/Pipfile.lock
    COMMAND ${PipEnv} run python ${SCRIPT_FILE} -r ${size} -o ${filename} VERBATIM
    WORKING_DIRECTORY ${SCRIPT_FOLDER}
  )

endfunction()

INCLUDE(FindPackageHandleStandardArgs)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(PythonResources REQUIRED_VARS PipEnv)
