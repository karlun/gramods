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
  ADD_CUSTOM_COMMAND(
    OUTPUT ${arg2}
    COMMAND ${Inkscape_EXECUTABLE} --export-area-page --export-png=${arg2} ${arg1}
    )
endfunction()

INCLUDE(FindPackageHandleStandardArgs)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(Inkscape REQUIRED_VARS Inkscape_EXECUTABLE)

