
#include <gtest/gtest.h>

#include "base_config_functionality.cpp"
#include "load_empty_lib_config_file.cpp"


int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  int ret = RUN_ALL_TESTS();
  return ret;
}
