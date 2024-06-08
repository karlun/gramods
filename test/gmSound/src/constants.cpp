
#include <gmSound/SoundConstants.hh>

#include <gmCore/Console.hh>
#include <gmCore/OStreamMessageSink.hh>

using namespace gramods;

TEST(gmSoundConstants, PolyFit) {

  gmCore::Console::removeAllSinks();
#if 0
  std::shared_ptr<gmCore::OStreamMessageSink> osms =
    std::make_shared<gmCore::OStreamMessageSink>();
  osms->setUseAnsiColor(true);
  osms->setLevel(4);
  osms->initialize();
#elif 0
  std::shared_ptr<gmCore::NullMessageSink> nullsink =
    std::make_shared<gmCore::NullMessageSink>();
  nullsink->initialize();
#endif

  EXPECT_LE(std::fabs(SPEED_OF_SOUND.getValue({35})[0] - 351.88), 1e-2);
  EXPECT_LE(std::fabs(SPEED_OF_SOUND.getValue({30})[0] - 349.02), 1e-2);
  EXPECT_LE(std::fabs(SPEED_OF_SOUND.getValue({25})[0] - 346.13), 1e-2);
  EXPECT_LE(std::fabs(SPEED_OF_SOUND.getValue({20})[0] - 343.21), 1e-2);
  EXPECT_LE(std::fabs(SPEED_OF_SOUND.getValue({15})[0] - 340.27), 1e-2);
  EXPECT_LE(std::fabs(SPEED_OF_SOUND.getValue({10})[0] - 337.31), 1e-2);
  EXPECT_LE(std::fabs(SPEED_OF_SOUND.getValue({5})[0] - 334.32), 1e-2);
  EXPECT_LE(std::fabs(SPEED_OF_SOUND.getValue({0})[0] - 331.30), 1e-2);
  EXPECT_LE(std::fabs(SPEED_OF_SOUND.getValue({-5})[0] - 328.25), 1e-2);
  EXPECT_LE(std::fabs(SPEED_OF_SOUND.getValue({-10})[0] - 325.18), 1e-2);
  EXPECT_LE(std::fabs(SPEED_OF_SOUND.getValue({-15})[0] - 322.07), 1e-2);
  EXPECT_LE(std::fabs(SPEED_OF_SOUND.getValue({-20})[0] - 318.94), 1e-2);
  EXPECT_LE(std::fabs(SPEED_OF_SOUND.getValue({-25})[0] - 315.77), 1e-2);
}
