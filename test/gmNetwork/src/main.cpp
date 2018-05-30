
#include <gtest/gtest.h>

#define gramods_STRIP_PATH_FROM_FILE
#include "basic.cpp"

#include <gmCore/OStreamMessageSink.hh>

int main(int argc, char **argv) {
  std::shared_ptr<gmCore::OStreamMessageSink> osms =
    std::make_shared<gmCore::OStreamMessageSink>();
  osms->initialize();

  ::testing::InitGoogleTest(&argc, argv);
  int ret = RUN_ALL_TESTS();
  return ret;
}
