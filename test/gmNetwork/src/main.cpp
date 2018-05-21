
#include <gtest/gtest.h>

#define gramods_STRIP_PATH_FROM_FILE
#include "basic.cpp"


int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  int ret = RUN_ALL_TESTS();
  return ret;
}
