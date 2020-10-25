#
# This module defines
# INKSCAPE_EXECUTABLE
#
# And function
# convert_svg_to_png(filename.svg, filename.png)
#

find_program(Inkscape_EXECUTABLE
  NAMES inkscape
  PATHS
  /usr/bin
  /usr/local/bin
  C:/Program Files/
  )

function (convert_svg_to_png arg1 arg2)

  IF (NOT INKSCAPE_VERSION)
    EXECUTE_PROCESS(COMMAND inkscape --version OUTPUT_VARIABLE INKSCAPE_VERSION)
  ENDIF()

  IF ("${INKSCAPE_VERSION}" MATCHES " 0\\.9[0-2]")
    ADD_CUSTOM_COMMAND(
      OUTPUT ${arg2}
      DEPENDS ${arg1}
      COMMAND ${Inkscape_EXECUTABLE} --export-area-page --export-png=${arg2} ${arg1}
      )
  ELSE()
    ADD_CUSTOM_COMMAND(
      OUTPUT ${arg2}
      DEPENDS ${arg1}
      COMMAND ${Inkscape_EXECUTABLE} --export-area-page --export-filename=${arg2} ${arg1}
      )
  ENDIF()
endfunction()

INCLUDE(FindPackageHandleStandardArgs)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(Inkscape REQUIRED_VARS Inkscape_EXECUTABLE)

