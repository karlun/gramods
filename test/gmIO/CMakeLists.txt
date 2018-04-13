CMAKE_MINIMUM_REQUIRED(VERSION 2.8.3)


SET (TEST_INCLUDE_DIRS CACHE INTERNAL "The list of include folders to search for the io test.")
SET (TEST_LIBS CACHE INTERNAL "The list of libraries that the io test should link against.")

SET (TEST_INCLUDE_DIRS ${TEST_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/include)

IF (TARGET gmIO)
  SET (TEST_LIBS ${TEST_LIBS} gmIO)
ELSE ()
  MESSAGE("gmIO test cannot be built without gmIO module")
  RETURN()
ENDIF ()

FIND_PACKAGE(GTest)
IF (GTest_FOUND)
  SET (TEST_INCLUDE_DIRS ${TEST_INCLUDE_DIRS} ${GTEST_INCLUDE_DIRS})
  SET (TEST_LIBS ${TEST_LIBS} ${GTEST_BOTH_LIBRARIES})
ELSE ()
  MESSAGE("gmIO test cannot be built without gtest")
  RETURN()
ENDIF ()

FIND_PACKAGE(Threads)
IF (Threads_FOUND)
  SET (TEST_LIBS ${TEST_LIBS} ${CMAKE_THREAD_LIBS_INIT})
ELSE ()
  MESSAGE("gmIO test cannot be built without threads")
  RETURN()
ENDIF ()

ADD_EXECUTABLE(test_gmIO src/main.cpp)

TARGET_INCLUDE_DIRECTORIES(test_gmIO PUBLIC ${TEST_INCLUDE_DIRS})
TARGET_LINK_LIBRARIES(test_gmIO ${TEST_LIBS})

SET_PROPERTY(TARGET test_gmIO PROPERTY CXX_STANDARD 14)
