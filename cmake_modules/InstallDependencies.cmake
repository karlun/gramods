
function(install_dependencies EXEC_FILES LIB_FILES)
  CMAKE_MINIMUM_REQUIRED(VERSION 3.14)
  CMAKE_POLICY(VERSION 3.14)

  # Path to search for third-party dependencies. This should include
  # vcpkg if correctly configured
  INSTALL(CODE "LIST(APPEND DEP_FOLDERS \"${CMAKE_PREFIX_PATH}\")")
  INSTALL(CODE "LIST(APPEND DEP_FOLDERS \"${CMAKE_LIBRARY_PATH}\")")
  INSTALL(CODE "LIST(APPEND DEP_FOLDERS \"${CMAKE_CURRENT_SOURCE_DIR}\")")
  INSTALL(CODE "SET(EXEC_FILES \"${EXEC_FILES}\")")
  INSTALL(CODE "SET(LIB_FILES \"${LIB_FILES}\")")

  INSTALL(CODE [[

    LIST(APPEND pre_exclude_regexes "api-ms-.*") # windows API
    LIST(APPEND pre_exclude_regexes "ext-ms-.*") # windows API
    LIST(APPEND post_exclude_regexes ".*WINDOWS[\\/]system32.*") # windows system dlls

    FILE(GET_RUNTIME_DEPENDENCIES
      EXECUTABLES ${EXEC_FILES}
      LIBRARIES ${LIB_FILES}
      RESOLVED_DEPENDENCIES_VAR _r_deps
      UNRESOLVED_DEPENDENCIES_VAR _u_deps
      CONFLICTING_DEPENDENCIES_PREFIX _c_deps
      DIRECTORIES ${DEP_FOLDERS}
      PRE_EXCLUDE_REGEXES ${pre_exclude_regexes}
      POST_EXCLUDE_REGEXES ${post_exclude_regexes}
      )

    IF (_u_deps)
      MESSAGE(WARNING "There were unresolved dependencies: \"${_u_deps}\"!")
    ENDIF()
    FOREACH (file ${_c_deps_FILENAMES})
      MESSAGE(WARNING "There were conflicting dependencies for ${file}: ${_c_deps_${file}}!")
    ENDFOREACH ()

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
