
#include <gtest/gtest.h>

#include "openal.cpp"
#include "multilateration.cpp"

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  int ret = RUN_ALL_TESTS();
  return ret;
}
