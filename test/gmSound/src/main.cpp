
#include <gtest/gtest.h>

#include "constants.cpp"
#include "openal.cpp"
#include "multilateration.cpp"

#include <test/config.hh>

int main(int argc, char **argv) {
  gramods::test::addConsoleMessageSinks();

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
