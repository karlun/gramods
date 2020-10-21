
#include <gtest/gtest.h>

#define gramods_STRIP_PATH_FROM_FILE

#include "angle.cpp"
#include "eigen.cpp"

#include "base_config_functionality.cpp"
#include "load_empty_lib_config_file.cpp"
#include "console.cpp"


int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  int ret = RUN_ALL_TESTS();
  return ret;
}
