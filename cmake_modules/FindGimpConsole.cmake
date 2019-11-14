#
# This module defines
# GimpConsole_EXECUTABLE
#
# And function
# gimp_build_image(scriptname, filename.png)
#

find_program(GimpConsole_EXECUTABLE
  NAMES gimp-console
  PATHS
  /usr/bin
  /usr/local/bin
  C:/Program Files/
  )

function (gimp_build_image script filename size)
  GET_FILENAME_COMPONENT(script_path ${script} DIRECTORY)
  GET_FILENAME_COMPONENT(script_name ${script} NAME_WE)
  ADD_CUSTOM_COMMAND(
    OUTPUT ${filename}
    DEPENDS ${script}
    COMMAND ${GimpConsole_EXECUTABLE} -idf --batch-interpreter python-fu-eval -b "import sys; sys.dont_write_bytecode = True; sys.path = ['${script_path}'] + sys.path; import ${script_name}; ${script_name}.run(size = ${size}, output='${filename}')" -b "pdb.gimp_quit(1)" VERBATIM
    )
endfunction()

INCLUDE(FindPackageHandleStandardArgs)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(GimpConsole REQUIRED_VARS GimpConsole_EXECUTABLE)
