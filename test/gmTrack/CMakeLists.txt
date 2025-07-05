CMAKE_MINIMUM_REQUIRED(VERSION 3.10)
CMAKE_POLICY(VERSION 3.10)


SET (TEST_INCLUDE_DIRS CACHE INTERNAL "The list of include folders to search for the track test.")
SET (TEST_LIBS CACHE INTERNAL "The list of libraries that the track test should link against.")

SET(gramods_PARTS
  ${gramods_PARTS} "test_gmTrack" PARENT_SCOPE)

LIST (APPEND TEST_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include)

IF (NOT TARGET gmTrack)
  SET(gramods_REQ_test_gmTrack "gmTrack" PARENT_SCOPE)
  RETURN()
ENDIF ()
LIST (APPEND TEST_LIBS gmTrack_internal_deps)

FIND_PACKAGE(GTest REQUIRED)
LIST (APPEND TEST_LIBS GTest::GTest)

FIND_PACKAGE(VRPN CONFIG QUIET)
IF(VRPN_FOUND)

  SET(HAVE_VRPN 1)
  OPTION(gramods_ENABLE_VRPN "Enable functionality that requires VRPN" ON)

  SET(HAVE_VRPN_server 1)
  OPTION(gramods_ENABLE_VRPN_server "Enable functionality that requires VRPN_server" ON)

  IF (gramods_ENABLE_VRPN)
    LIST (APPEND PRIVATE_LIBS VRPN::Client)
    LIST(APPEND gramods_FEAT_test_gmTrack VRPNClient)
  ELSE()
    LIST(APPEND gramods_DISA_test_gmTrack VRPNClient)
  ENDIF()

  IF (gramods_ENABLE_VRPN_server)
    LIST (APPEND PRIVATE_LIBS VRPN::Server)
    LIST(APPEND gramods_FEAT_test_gmTrack VRPNServer)
  ELSE()
    LIST(APPEND gramods_DISA_test_gmTrack VRPNServer)
  ENDIF()

  IF (gramods_ENABLE_VRPN OR gramods_ENABLE_VRPN_server)
    LIST (APPEND PRIVATE_LIBS VRPN::Quat)
  ENDIF()
ELSE()
  LIST(APPEND gramods_MISS_test_gmTrack VRPN)
ENDIF(VRPN_FOUND)

FIND_PACKAGE(Threads QUIET)
IF (NOT Threads_FOUND)
  SET(gramods_REQ_test_gmTrack "Threads" PARENT_SCOPE)
  RETURN()
ENDIF ()
LIST (APPEND TEST_LIBS Threads::Threads)

ADD_EXECUTABLE(test_gmTrack src/main.cpp)
SET(gramods_FEAT_test_gmTrack ${gramods_FEAT_test_gmTrack} PARENT_SCOPE)
SET(gramods_DISA_test_gmTrack ${gramods_DISA_test_gmTrack} PARENT_SCOPE)
SET(gramods_MISS_test_gmTrack ${gramods_MISS_test_gmTrack} PARENT_SCOPE)

TARGET_INCLUDE_DIRECTORIES(test_gmTrack PUBLIC ${TEST_INCLUDE_DIRS})
TARGET_LINK_LIBRARIES(test_gmTrack ${TEST_LIBS})

INCLUDE(CTest)
ADD_TEST(NAME TestGramodsTrack COMMAND test_gmTrack)

SET_PROPERTY(TARGET test_gmTrack PROPERTY CXX_STANDARD 20)
