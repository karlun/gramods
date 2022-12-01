CMAKE_MINIMUM_REQUIRED(VERSION 3.10)
CMAKE_POLICY(VERSION 3.10)


SET (TEST_INCLUDE_DIRS CACHE INTERNAL "The list of include folders to search for the touch test.")
SET (TEST_LIBS CACHE INTERNAL "The list of libraries that the touch test should link against.")

LIST (APPEND TEST_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include)

IF (NOT TARGET gmCore)
  MESSAGE(STATUS "gmTouch test will not be built without gmCore module")
  RETURN()
ENDIF ()
LIST (APPEND TEST_LIBS gmCore_internal_deps)

IF (NOT TARGET gmTouch)
  MESSAGE(STATUS "gmTouch test will not be built without gmTouch module")
  RETURN()
ENDIF ()
LIST (APPEND TEST_LIBS gmTouch_internal_deps)

FIND_PACKAGE(GTest)
IF (NOT GTest_FOUND)
  MESSAGE(STATUS "gmTouch test will not be built without gtest")
  RETURN()
ENDIF ()
LIST (APPEND TEST_INCLUDE_DIRS ${GTEST_INCLUDE_DIRS})
LIST (APPEND TEST_LIBS ${GTEST_BOTH_LIBRARIES})

FIND_PACKAGE(Threads)
IF (NOT Threads_FOUND)
  MESSAGE(STATUS "gmTouch test will not be built without threads")
  RETURN()
ENDIF ()
LIST (APPEND TEST_LIBS Threads::Threads)

ADD_EXECUTABLE(test_gmTouch src/main.cpp)

TARGET_INCLUDE_DIRECTORIES(test_gmTouch PUBLIC ${TEST_INCLUDE_DIRS})
TARGET_LINK_LIBRARIES(test_gmTouch ${TEST_LIBS})

INCLUDE(CTest)
ADD_TEST(NAME TestGramodsTouch COMMAND test_gmTouch)

SET_PROPERTY(TARGET test_gmTouch PROPERTY CXX_STANDARD 17)
