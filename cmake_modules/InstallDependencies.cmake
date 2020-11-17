
function(install_executable_dependencies EXEC_FILE)
  IF (NOT gramods_INSTALL_DEPENDENCIES)
    RETURN()
  ENDIF()
  IF (NOT EXEC_FILE)
    MESSAGE(WARNING "install_executable_dependencies called without path to executable")
    RETURN()
  ENDIF()

  # Path to search for third-party dependencies. This should include
  # vcpkg if correctly configured
  INSTALL(CODE "LIST(APPEND DEP_FOLDERS \"${CMAKE_PREFIX_PATH}\")")
  INSTALL(CODE "LIST(APPEND DEP_FOLDERS \"${CMAKE_LIBRARY_PATH}\")")
  INSTALL(CODE "LIST(APPEND DEP_FOLDERS \"${CMAKE_CURRENT_SOURCE_DIR}\")")
  INSTALL(CODE "SET(EXEC_FILE \"${EXEC_FILE}\")")

  INSTALL(CODE [[

    FILE(GET_RUNTIME_DEPENDENCIES
      EXECUTABLES ${EXEC_FILE}
      RESOLVED_DEPENDENCIES_VAR _r_deps
      UNRESOLVED_DEPENDENCIES_VAR _u_deps
      CONFLICTING_DEPENDENCIES_PREFIX _c_deps
      DIRECTORIES ${DEP_FOLDERS}
      PRE_EXCLUDE_REGEXES "api-ms-*"
      POST_EXCLUDE_REGEXES ".*system32/.*\\.dll" ".*SysWOW64/.*\\.dll"
      )

    IF (_u_deps)
      MESSAGE(WARNING "There were unresolved dependencies for executable ${EXEC_FILE}: \"${_u_deps}\"!")
    ENDIF()
    IF (_c_deps_FILENAMES)
      MESSAGE(WARNING "There were conflicting dependencies for executable ${EXEC_FILE}: \"${_c_deps_FILENAMES}\"!")
    ENDIF()

    FOREACH(_file ${_r_deps})
      FILE(INSTALL
        DESTINATION "${CMAKE_INSTALL_PREFIX}/bin"
        TYPE SHARED_LIBRARY
        FOLLOW_SYMLINK_CHAIN
        FILES "${_file}"
        )
    ENDFOREACH()
    ]])

endfunction()

function(install_library_dependencies LIB_FILE)
  IF (NOT gramods_INSTALL_DEPENDENCIES)
    RETURN()
  ENDIF()
  IF (NOT LIB_FILE)
    MESSAGE(WARNING "install_library_dependencies called without path to library")
    RETURN()
  ENDIF()

  # Path to search for third-party dependencies. This should include
  # vcpkg if correctly configured
  INSTALL(CODE "LIST(APPEND DEP_FOLDERS \"${CMAKE_PREFIX_PATH}\")")
  INSTALL(CODE "LIST(APPEND DEP_FOLDERS \"${CMAKE_LIBRARY_PATH}\")")
  INSTALL(CODE "LIST(APPEND DEP_FOLDERS \"${CMAKE_CURRENT_SOURCE_DIR}\")")
  INSTALL(CODE "SET(LIB_FILE \"${LIB_FILE}\")")

  INSTALL(CODE [[

    FILE(GET_RUNTIME_DEPENDENCIES
      LIBRARIES ${LIB_FILE}
      RESOLVED_DEPENDENCIES_VAR _r_deps
      UNRESOLVED_DEPENDENCIES_VAR _u_deps
      CONFLICTING_DEPENDENCIES_PREFIX _c_deps
      DIRECTORIES ${DEP_FOLDERS}
      PRE_EXCLUDE_REGEXES "api-ms-*"
      POST_EXCLUDE_REGEXES ".*system32/.*\\.dll"
      )

    IF (_u_deps)
      MESSAGE(WARNING "There were unresolved dependencies for library ${LIB_FILE}: \"${_u_deps}\"!")
    ENDIF()
    IF (_c_deps_FILENAMES)
      MESSAGE(WARNING "There were conflicting dependencies for library ${LIB_FILE}: \"${_c_deps}\"!")
    ENDIF()

    IF (WIN32)
      SET(LIB_FOLDER "bin")
    ELSE()
      SET(LIB_FOLDER "lib")
    ENDIF()

    FOREACH(_file ${_r_deps})
      FILE(INSTALL
        DESTINATION "${CMAKE_INSTALL_PREFIX}/${LIB_FOLDER}"
        TYPE SHARED_LIBRARY
        FOLLOW_SYMLINK_CHAIN
        FILES "${_file}"
        )
    ENDFOREACH()
    ]])

endfunction()
