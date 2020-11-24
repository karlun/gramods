# This module defines
# GIT_BRANCH
# GIT_COMMIT_HASH
# GIT_COMMIT_DATE
# GIT_COMMIT_DATE_ISO8601

find_package(Git)
if (NOT Git_FOUND)
  message(WARNING "No Git executable found - cannot extract and use Git data")
  return()
endif()

execute_process(
  COMMAND ${GIT_EXECUTABLE} rev-parse --abbrev-ref HEAD
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  RESULT_VARIABLE err
  OUTPUT_VARIABLE GIT_BRANCH
  OUTPUT_STRIP_TRAILING_WHITESPACE
  )

if (${err})
  message(WARNING "Git command failed")
  return()
endif()

execute_process(
  COMMAND ${GIT_EXECUTABLE} log -1 --format=%h
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  RESULT_VARIABLE err
  OUTPUT_VARIABLE GIT_COMMIT_HASH
  OUTPUT_STRIP_TRAILING_WHITESPACE
  )

if (${err})
  message(WARNING "Git command failed")
  return()
endif()

execute_process(
  COMMAND ${GIT_EXECUTABLE} log -1 --format=%cs
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  RESULT_VARIABLE err
  OUTPUT_VARIABLE GIT_COMMIT_DATE
  OUTPUT_STRIP_TRAILING_WHITESPACE
  )

execute_process(
  COMMAND ${GIT_EXECUTABLE} log -1 --format=%cI
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  RESULT_VARIABLE err
  OUTPUT_VARIABLE GIT_COMMIT_DATE_ISO8601
  OUTPUT_STRIP_TRAILING_WHITESPACE
  )

if (${err})
  message(WARNING "Git command failed")
  return()
endif()

