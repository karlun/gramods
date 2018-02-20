
#include <gtest/gtest.h>

#include "load_empty_lib_config_file.cpp"


int main(int argc, char **argv) {
  std::cerr << __FILE__ << ":" << __LINE__ << std::endl;
  ::testing::InitGoogleTest(&argc, argv);
  std::cerr << __FILE__ << ":" << __LINE__ << std::endl;
  int ret = RUN_ALL_TESTS();
  std::cerr << __FILE__ << ":" << __LINE__ << std::endl;
  return ret;
}
