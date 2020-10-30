CMAKE_MINIMUM_REQUIRED(VERSION 3.10)
CMAKE_POLICY(VERSION 3.10)


SET (TEST_INCLUDE_DIRS CACHE INTERNAL "The list of include folders to search for the track test.")
SET (TEST_LIBS CACHE INTERNAL "The list of libraries that the track test should link against.")

LIST (APPEND TEST_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include)

IF (NOT TARGET gmTrack)
  MESSAGE(STATUS "gmTrack test will not be built without gmTrack module")
  RETURN()
ENDIF ()
LIST (APPEND TEST_LIBS gmTrack)

FIND_PACKAGE(GTest)
IF (NOT GTest_FOUND)
  MESSAGE(STATUS "gmTrack test will not be built without gtest")
  RETURN()
ENDIF ()
LIST (APPEND TEST_INCLUDE_DIRS ${GTEST_INCLUDE_DIRS})
LIST (APPEND TEST_LIBS ${GTEST_BOTH_LIBRARIES})

FIND_PACKAGE(Threads)
IF (NOT Threads_FOUND)
  MESSAGE(STATUS "gmTrack test will not be built without threads")
  RETURN()
ENDIF ()
LIST (APPEND TEST_LIBS Threads::Threads)

ADD_EXECUTABLE(test_gmTrack src/main.cpp)

TARGET_INCLUDE_DIRECTORIES(test_gmTrack PUBLIC ${TEST_INCLUDE_DIRS})
TARGET_LINK_LIBRARIES(test_gmTrack ${TEST_LIBS})

INCLUDE(CTest)
ADD_TEST(NAME TestGramodsTrack COMMAND test_gmTrack)

SET_PROPERTY(TARGET test_gmTrack PROPERTY CXX_STANDARD 14)
