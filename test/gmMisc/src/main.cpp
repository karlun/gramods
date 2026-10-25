
#include <gtest/gtest.h>

#include "accumulative_mean.cpp"
#include "end-fit.cpp"
#include "fftw.cpp"
#include "polyfit.cpp"
#include "neldermead.cpp"
#include "delaunay.cpp"

#include <test/config.hh>

int main(int argc, char **argv) {
  gramods::test::addConsoleMessageSinks();

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
