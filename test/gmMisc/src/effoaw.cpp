
#include <gmMisc/EFFOAW.hh>

using namespace gramods;

TEST(gmMiscEffoaw, Effoaw) {

  gmMisc::EFFOAW effoaw;
  effoaw.setHistoryLength(6);
  effoaw.setHistoryDuration(10);

  effoaw.addSample(0, { 9, 0 }, 1);
  effoaw.addSample(0, { 0, 1 }, 2);
  effoaw.addSample(0, { 5, 2 }, 3);
  effoaw.addSample(0, { 0, 3 }, 4);
  effoaw.addSample(0, { 1, 4 }, 5);
  effoaw.addSample(0, { 0, 5 }, 6);
  effoaw.addSample(0, { 0, 6 }, 7);

  {
    size_t samples;
    auto vel = effoaw.estimateVelocity(0, 20, &samples);
    EXPECT_EQ(samples, 6);
  }

  {
    size_t samples;
    auto vel = effoaw.estimateVelocity(0, 4, &samples);
    EXPECT_EQ(samples, 5);
  }

  {
    size_t samples;
    auto vel = effoaw.estimateVelocity(0, 3, &samples);
    EXPECT_EQ(samples, 4);
  }

  {
    size_t samples;
    auto vel = effoaw.estimateVelocity(0, 0.1, &samples);
    EXPECT_EQ(samples, 2);

    EXPECT_EQ(vel[0], 0);
    EXPECT_EQ(vel[1], 1);
  }
}

