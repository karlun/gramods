
#include <gmSound/OpenALContext.hh>
#include <gmSound/OpenALCapture.hh>

using namespace gramods;

TEST(gmSoundOpenAL, Context) {

  gmSound::OpenALContext context;
  auto names = context.getDeviceNames();
  EXPECT_GE(names.size(), 1);

  context.initialize();
  EXPECT_TRUE(context.isOpen());
}

TEST(gmSoundOpenAL, Capture) {

  gmSound::OpenALCapture capture;
  auto names = capture.getCaptureDeviceNames();
  EXPECT_GE(names.size(), 1);

  capture.initialize();
  EXPECT_TRUE(capture.isOpen());
}
