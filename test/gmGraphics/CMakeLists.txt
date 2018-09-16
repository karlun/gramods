CMAKE_MINIMUM_REQUIRED(VERSION 2.8.3)


SET (TEST_INCLUDE_DIRS CACHE INTERNAL "The list of include folders to search for the test.")
SET (TEST_LIBS CACHE INTERNAL "The list of libraries that the test should link against.")

SET (TEST_INCLUDE_DIRS ${TEST_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/include)


IF (TARGET gmGraphics)
  SET (TEST_LIBS ${TEST_LIBS} gmGraphics)
ELSE ()
  MESSAGE(STATUS "gmGraphics test will not be built without gmGraphics module")
  RETURN()
ENDIF ()

FIND_PACKAGE(GTest)
IF (GTest_FOUND)
  SET (TEST_INCLUDE_DIRS ${TEST_INCLUDE_DIRS} ${GTEST_INCLUDE_DIRS})
  SET (TEST_LIBS ${TEST_LIBS} ${GTEST_BOTH_LIBRARIES})
ELSE ()
  MESSAGE(STATUS "gmGraphics test will not be built without gtest")
  RETURN()
ENDIF ()

FIND_PACKAGE(Threads)
IF (Threads_FOUND)
  SET (TEST_LIBS ${TEST_LIBS} ${CMAKE_THREAD_LIBS_INIT})
ELSE ()
  MESSAGE(STATUS "gmGraphics test will not be built without threads")
  RETURN()
ENDIF ()

ADD_EXECUTABLE(test_gmGraphics src/main.cpp)

TARGET_INCLUDE_DIRECTORIES(test_gmGraphics PUBLIC ${TEST_INCLUDE_DIRS})
TARGET_LINK_LIBRARIES(test_gmGraphics ${TEST_LIBS})

INCLUDE(CTest)
ADD_TEST(NAME TestGramods COMMAND test_gmGraphics)

SET_PROPERTY(TARGET test_gmGraphics PROPERTY CXX_STANDARD 14)