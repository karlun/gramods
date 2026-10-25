
#include <gtest/gtest.h>

#define gramods_STRIP_PATH_FROM_FILE
#include "sdl_window.cpp"
#include "intersection.cpp"

#include <test/config.hh>

int main(int argc, char **argv) {
  gramods::test::addConsoleMessageSinks();

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
