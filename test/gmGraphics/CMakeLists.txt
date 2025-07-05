CMAKE_MINIMUM_REQUIRED(VERSION 3.10)
CMAKE_POLICY(VERSION 3.10)


SET (TEST_INCLUDE_DIRS CACHE INTERNAL "The list of include folders to search for the test.")
SET (TEST_LIBS CACHE INTERNAL "The list of libraries that the test should link against.")

SET(gramods_PARTS
  ${gramods_PARTS} "test_gmGraphics" PARENT_SCOPE)

LIST (APPEND TEST_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include)

IF (NOT TARGET gmGraphics)
  SET(gramods_REQ_test_gmGraphics "gmGraphics" PARENT_SCOPE)
  RETURN()
ENDIF ()
LIST (APPEND TEST_LIBS gmGraphics_internal_deps)

FIND_PACKAGE(GTest REQUIRED)
LIST (APPEND TEST_LIBS GTest::GTest)

FIND_PACKAGE(Threads QUIET)
IF (NOT Threads_FOUND)
  SET(gramods_REQ_test_gmGraphics "Threads" PARENT_SCOPE)
  RETURN()
ENDIF ()
LIST (APPEND TEST_LIBS Threads::Threads)

ADD_EXECUTABLE(test_gmGraphics src/main.cpp)

TARGET_INCLUDE_DIRECTORIES(test_gmGraphics PUBLIC ${TEST_INCLUDE_DIRS})
TARGET_LINK_LIBRARIES(test_gmGraphics ${TEST_LIBS})

INCLUDE(CTest)
ADD_TEST(NAME TestGramodsGraphics COMMAND test_gmGraphics)

SET_PROPERTY(TARGET test_gmGraphics PROPERTY CXX_STANDARD 20)
