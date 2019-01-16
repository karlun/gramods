CMAKE_MINIMUM_REQUIRED(VERSION 2.8.3)


SET (TEST_INCLUDE_DIRS CACHE INTERNAL "The list of include folders to search for the core test.")
SET (TEST_LIBS CACHE INTERNAL "The list of libraries that the core test should link against.")

SET (TEST_INCLUDE_DIRS ${TEST_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/include)

IF (TARGET gmCore)
  SET (TEST_LIBS ${TEST_LIBS} gmCore)
ELSE ()
  MESSAGE(STATUS "gmCore test will not be built without gmCore module")
  RETURN()
ENDIF ()

FIND_PACKAGE(GTest)
IF (GTest_FOUND)
  SET (TEST_INCLUDE_DIRS ${TEST_INCLUDE_DIRS} ${GTEST_INCLUDE_DIRS})
  SET (TEST_LIBS ${TEST_LIBS} ${GTEST_BOTH_LIBRARIES})
ELSE ()
  MESSAGE(STATUS "gmCore test will not be built without gtest")
  RETURN()
ENDIF ()

FIND_PACKAGE(Threads)
IF (Threads_FOUND)
  SET (TEST_LIBS ${TEST_LIBS} ${CMAKE_THREAD_LIBS_INIT})
ELSE ()
  MESSAGE(STATUS "gmCore test will not be built without threads")
  RETURN()
ENDIF ()

ADD_EXECUTABLE(test_gmCore src/main.cpp)

TARGET_INCLUDE_DIRECTORIES(test_gmCore PUBLIC ${TEST_INCLUDE_DIRS})
TARGET_LINK_LIBRARIES(test_gmCore ${TEST_LIBS})

INCLUDE(CTest)
ADD_TEST(NAME TestGramodsCore COMMAND test_gmCore)

SET_PROPERTY(TARGET test_gmCore PROPERTY CXX_STANDARD 14)
