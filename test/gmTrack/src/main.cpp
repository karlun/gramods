
#include <gtest/gtest.h>

#define gramods_STRIP_PATH_FROM_FILE
#include "base_estimation.cpp"
#include "key_change_tracker.cpp"
#include "projection_texture.cpp"
#include "registered_tracker.cpp"
#include "vrpn.cpp"

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  int ret = RUN_ALL_TESTS();
  return ret;
}
