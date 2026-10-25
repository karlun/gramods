
#include <gtest/gtest.h>

#define gramods_STRIP_PATH_FROM_FILE
#include "runsync.cpp"
#include "datasync.cpp"

#include <test/config.hh>

int main(int argc, char **argv) {
  gramods::test::addConsoleMessageSinks();

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
