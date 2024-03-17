
#include <gtest/gtest.h>

#include "end-fit.cpp"
#include "fftw.cpp"
#include "polyfit.cpp"
#include "neldermead.cpp"
#include "delaunay.cpp"

#include <gmCore/OStreamMessageSink.hh>
#include <gmCore/LogFileMessageSink.hh>
#include <gmCore/NullMessageSink.hh>

using namespace gramods;

int main(int argc, char **argv) {
  gmCore::Console::removeAllSinks();
#if 0
  std::shared_ptr<gmCore::OStreamMessageSink> osms =
    std::make_shared<gmCore::OStreamMessageSink>();
  osms->setUseAnsiColor(true);
  osms->setLevel(4);
  osms->initialize();

  std::shared_ptr<gmCore::LogFileMessageSink> lfms =
    std::make_shared<gmCore::LogFileMessageSink>();
  lfms->setLogFilePath("gramods.log");
  lfms->setLevel(6);
  lfms->initialize();
#else
  std::shared_ptr<gmCore::NullMessageSink> nullsink =
    std::make_shared<gmCore::NullMessageSink>();
  nullsink->initialize();
#endif

  ::testing::InitGoogleTest(&argc, argv);
  int ret = RUN_ALL_TESTS();
  return ret;
}
