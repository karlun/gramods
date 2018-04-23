# This module defines
# PlantUML_JARFILE, the name of the jar file
# PlantUML_FOUND, if false, do not try to use PlantUML
#
# Compile with 'PLANTUML_DIR' to use PlantUML.

find_file(PlantUML_JARFILE
  NAMES plantuml.jar
  HINTS "" ENV PLANTUML_DIR
  )
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  PlantUML DEFAULT_MSG PlantUML_JARFILE)
