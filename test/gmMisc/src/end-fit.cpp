
#include <gmMisc/EFFOAW.hh>
#include <gmMisc/EFHOAW.hh>

#ifdef gramods_ENABLE_Eigen3

using namespace gramods;

TEST(gmMiscEndFit, Effoaw) {

  gmMisc::EFFOAW effoaw;
  effoaw.setHistoryLength(6);
  effoaw.setHistoryDuration(10);

  effoaw.addSample(0, { 9, 0, 0 }, 1);
  effoaw.addSample(0, { 0, 1, 0 }, 2);
  effoaw.addSample(0, { 5, 2, 0 }, 3);
  effoaw.addSample(0, { 0, 3, 0 }, 4);
  effoaw.addSample(0, { 1, 4, 0 }, 5);
  effoaw.addSample(0, { 0, 5, 0 }, 6);
  effoaw.addSample(0, { 0, 6, 0 }, 7);
  effoaw.cleanup();

  {
    size_t samples;
    effoaw.estimateVelocity(0, 20, &samples);
    EXPECT_EQ(samples, 6);
  }

  {
    size_t samples;
    effoaw.estimateVelocity(0, 4, &samples);
    EXPECT_EQ(samples, 5);
  }

  {
    size_t samples;
    effoaw.estimateVelocity(0, 3, &samples);
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

#define SAMPLES 100

TEST(gmMiscEndFit, Efhoaw) {

  gmMisc::EFHOAW efhoaw;
  efhoaw.setHistoryLength(SAMPLES);
  efhoaw.setHistoryDuration(10);

  double a = 1.0 / SAMPLES;

  for (int idx = 0; idx < SAMPLES; ++idx) {
    double t = 2 * 3.141592 * idx * a;
    auto X = Eigen::Vector3d(cos(t), sin(t), t);
    efhoaw.addSample(0, X, a * idx);
  }
  efhoaw.cleanup();

  size_t samples;
  gmMisc::EFHOAW::polco pc = efhoaw.estimateCoefficients(0, 0.1, 2, &samples);
  EXPECT_EQ(samples, 39);
}

#endif
