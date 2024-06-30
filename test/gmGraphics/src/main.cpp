
#include <gtest/gtest.h>

#define gramods_STRIP_PATH_FROM_FILE
//#include "sdl_window.cpp"
#include "intersection.cpp"


int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  int ret = RUN_ALL_TESTS();
  return ret;
}
