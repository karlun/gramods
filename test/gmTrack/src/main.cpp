
#include <gtest/gtest.h>

#define gramods_STRIP_PATH_FROM_FILE
#include "base_estimation.cpp"
#include "key_change_tracker.cpp"
#include "projection_texture.cpp"
#include "registered_tracker.cpp"
#include "vrpn.cpp"

#include <test/config.hh>

int main(int argc, char **argv) {
  gramods::test::addConsoleMessageSinks();

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
