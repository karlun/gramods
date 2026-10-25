
#include <gmSound/SoundConstants.hh>

using namespace gramods;

TEST(gmSoundConstants, PolyFit) {

  EXPECT_LE(std::fabs(SPEED_OF_SOUND.getSingle(35) - 351.88), 1e-2);
  EXPECT_LE(std::fabs(SPEED_OF_SOUND.getSingle(30) - 349.02), 1e-2);
  EXPECT_LE(std::fabs(SPEED_OF_SOUND.getSingle(25) - 346.13), 1e-2);
  EXPECT_LE(std::fabs(SPEED_OF_SOUND.getSingle(20) - 343.21), 1e-2);
  EXPECT_LE(std::fabs(SPEED_OF_SOUND.getSingle(15) - 340.27), 1e-2);
  EXPECT_LE(std::fabs(SPEED_OF_SOUND.getSingle(10) - 337.31), 1e-2);
  EXPECT_LE(std::fabs(SPEED_OF_SOUND.getSingle(5) - 334.32), 1e-2);
  EXPECT_LE(std::fabs(SPEED_OF_SOUND.getSingle(0) - 331.30), 1e-2);
  EXPECT_LE(std::fabs(SPEED_OF_SOUND.getSingle(-5) - 328.25), 1e-2);
  EXPECT_LE(std::fabs(SPEED_OF_SOUND.getSingle(-10) - 325.18), 1e-2);
  EXPECT_LE(std::fabs(SPEED_OF_SOUND.getSingle(-15) - 322.07), 1e-2);
  EXPECT_LE(std::fabs(SPEED_OF_SOUND.getSingle(-20) - 318.94), 1e-2);
  EXPECT_LE(std::fabs(SPEED_OF_SOUND.getSingle(-25) - 315.77), 1e-2);
}
