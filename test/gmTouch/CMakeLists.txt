CMAKE_MINIMUM_REQUIRED(VERSION 3.10)
CMAKE_POLICY(VERSION 3.10)


SET (TEST_INCLUDE_DIRS CACHE INTERNAL "The list of include folders to search for the touch test.")
SET (TEST_LIBS CACHE INTERNAL "The list of libraries that the touch test should link against.")

SET(gramods_PARTS
  ${gramods_PARTS} "test_gmTouch" PARENT_SCOPE)

LIST (APPEND TEST_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include)

IF (NOT TARGET gmCore)
  SET(gramods_REQ_test_gmTouch "gmCore" PARENT_SCOPE)
  RETURN()
ENDIF ()
LIST (APPEND TEST_LIBS gmCore_internal_deps)

IF (NOT TARGET gmTouch)
  SET(gramods_REQ_test_gmTouch "gmTouch" PARENT_SCOPE)
  RETURN()
ENDIF ()
LIST (APPEND TEST_LIBS gmTouch_internal_deps)

FIND_PACKAGE(GTest REQUIRED)
LIST (APPEND TEST_LIBS GTest::GTest)

FIND_PACKAGE(Threads QUIET)
IF (NOT Threads_FOUND)
  SET(gramods_REQ_test_gmTouch "Threads" PARENT_SCOPE)
  RETURN()
ENDIF ()
LIST (APPEND TEST_LIBS Threads::Threads)

ADD_EXECUTABLE(test_gmTouch src/main.cpp)

TARGET_INCLUDE_DIRECTORIES(test_gmTouch PUBLIC ${TEST_INCLUDE_DIRS})
TARGET_LINK_LIBRARIES(test_gmTouch ${TEST_LIBS})

INCLUDE(CTest)
ADD_TEST(NAME TestGramodsTouch COMMAND test_gmTouch)

SET_PROPERTY(TARGET test_gmTouch PROPERTY CXX_STANDARD 17)
