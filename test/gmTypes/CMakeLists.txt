CMAKE_MINIMUM_REQUIRED(VERSION 2.8.3)


SET (TEST_INCLUDE_DIRS CACHE INTERNAL "The list of include folders to search for the core test.")
SET (TEST_LIBS CACHE INTERNAL "The list of libraries that the core test should link against.")

SET (TEST_INCLUDE_DIRS ${TEST_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR}/include)

IF (NOT TARGET gmTypes)
  MESSAGE(STATUS "gmTypes test will not be built without gmTypes module")
  RETURN()
ENDIF ()
SET (TEST_LIBS ${TEST_LIBS} gmTypes)

FIND_PACKAGE(GTest)
IF (NOT GTest_FOUND)
  MESSAGE(STATUS "gmCore test will not be built without gtest")
  RETURN()
ENDIF ()
SET (TEST_INCLUDE_DIRS ${TEST_INCLUDE_DIRS} ${GTEST_INCLUDE_DIRS})
SET (TEST_LIBS ${TEST_LIBS} ${GTEST_BOTH_LIBRARIES})

FIND_PACKAGE(Threads)
IF (NOT Threads_FOUND)
  MESSAGE(STATUS "gmCore test will not be built without threads")
  RETURN()
ENDIF ()
SET (TEST_LIBS ${TEST_LIBS} Threads::Threads)

ADD_EXECUTABLE(test_gmTypes src/main.cpp)

TARGET_INCLUDE_DIRECTORIES(test_gmTypes PUBLIC ${TEST_INCLUDE_DIRS})
TARGET_LINK_LIBRARIES(test_gmTypes ${TEST_LIBS})

INCLUDE(CTest)
ADD_TEST(NAME TestGramodsTypes COMMAND test_gmTypes)

SET_PROPERTY(TARGET test_gmTypes PROPERTY CXX_STANDARD 14)
