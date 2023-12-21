
#include <gtest/gtest.h>

#define gramods_STRIP_PATH_FROM_FILE
#include "vrpn.cpp"
#include "registered_tracker.cpp"
#include "base_estimation.cpp"
#include "projection_texture.cpp"
#include "mapper.cpp"

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  int ret = RUN_ALL_TESTS();
  return ret;
}
