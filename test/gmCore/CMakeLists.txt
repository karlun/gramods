CMAKE_MINIMUM_REQUIRED(VERSION 3.10)
CMAKE_POLICY(VERSION 3.10)


SET (TEST_INCLUDE_DIRS CACHE INTERNAL "The list of include folders to search for the core test.")
SET (TEST_LIBS CACHE INTERNAL "The list of libraries that the core test should link against.")

SET(gramods_PARTS
  ${gramods_PARTS} "test_gmCore" PARENT_SCOPE)

LIST (APPEND TEST_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include)

IF (NOT TARGET gmCore)
  SET(gramods_REQ_test_gmCore "gmCore" PARENT_SCOPE)
  RETURN()
ENDIF ()
LIST (APPEND TEST_LIBS gmCore_internal_deps)

FIND_PACKAGE(GTest REQUIRED)
LIST (APPEND TEST_LIBS GTest::GTest)

FIND_PACKAGE(Threads QUIET)
IF (NOT Threads_FOUND)
  SET(gramods_REQ_test_gmCore "Threads" PARENT_SCOPE)
  RETURN()
ENDIF ()
LIST (APPEND TEST_LIBS Threads::Threads)

ADD_EXECUTABLE(test_gmCore src/main.cpp)

TARGET_INCLUDE_DIRECTORIES(test_gmCore PUBLIC ${TEST_INCLUDE_DIRS})
TARGET_LINK_LIBRARIES(test_gmCore ${TEST_LIBS})

INCLUDE(CTest)
ADD_TEST(NAME TestGramodsCore COMMAND test_gmCore)

SET_PROPERTY(TARGET test_gmCore PROPERTY CXX_STANDARD 20)
