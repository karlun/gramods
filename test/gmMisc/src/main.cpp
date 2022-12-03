
#include <gtest/gtest.h>

#include "end-fit.cpp"
#include "polyfit.cpp"
#include "neldermead.cpp"
#include "delaunay.cpp"

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  int ret = RUN_ALL_TESTS();
  return ret;
}
