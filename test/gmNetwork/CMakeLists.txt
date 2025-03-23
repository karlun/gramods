CMAKE_MINIMUM_REQUIRED(VERSION 3.10)
CMAKE_POLICY(VERSION 3.10)


SET (TEST_INCLUDE_DIRS CACHE INTERNAL "The list of include folders to search for the network test.")
SET (TEST_LIBS CACHE INTERNAL "The list of libraries that the network test should link against.")

SET(gramods_PARTS
  ${gramods_PARTS} "test_gmNetwork" PARENT_SCOPE)

LIST (APPEND TEST_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include)

IF (NOT TARGET gmNetwork)
  SET(gramods_REQ_test_gmNetwork "gmNetwork" PARENT_SCOPE)
  RETURN()
ENDIF ()
LIST (APPEND TEST_LIBS gmNetwork)

FIND_PACKAGE(GTest REQUIRED)
LIST (APPEND TEST_LIBS GTest::GTest)

FIND_PACKAGE(Threads QUIET)
IF (NOT Threads_FOUND)
  SET(gramods_REQ_test_gmNetwork "Threads" PARENT_SCOPE)
  RETURN()
ENDIF ()
LIST (APPEND TEST_LIBS Threads::Threads)

ADD_EXECUTABLE(test_gmNetwork src/main.cpp)

TARGET_INCLUDE_DIRECTORIES(test_gmNetwork PUBLIC ${TEST_INCLUDE_DIRS})
TARGET_LINK_LIBRARIES(test_gmNetwork ${TEST_LIBS})

INCLUDE(CTest)
ADD_TEST(NAME TestGramodsNetwork COMMAND test_gmNetwork)

SET_PROPERTY(TARGET test_gmNetwork PROPERTY CXX_STANDARD 17)
