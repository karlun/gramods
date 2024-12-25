
#include <gmMisc/AccumulativeMean.hh>

#include <gmCore/MathConstants.hh>

using namespace gramods;

TEST(gmMiscAccumulativeMean, Float) {

  gmMisc::AccumulativeMean<float> mean;
  mean.setDefaultFunc();

  const size_t MAX = 1e7;
  for (size_t idx = 0; idx < MAX; ++idx) mean += float(idx);
  float est(mean);

  EXPECT_LE(std::abs(est - 0.5f * (MAX - 1)), 1e-8);
}

TEST(gmMiscAccumulativeMean, Double) {

  gmMisc::AccumulativeMean<double, double> mean;
  mean.setDefaultFunc();

  const size_t MAX = 1e7;
  for (size_t idx = 0; idx < MAX; ++idx) mean += double(idx);
  double est(mean);

  EXPECT_LE(std::abs(est - 0.5 * (MAX - 1)), 1e-8);
}

#ifdef gramods_ENABLE_Eigen3

#include <Eigen/Eigen>

TEST(gmMiscAccumulativeMean, EigenVector) {

  gmMisc::AccumulativeMean<Eigen::Vector3f> mean( //
      Eigen::Vector3f::Zero());
  mean.setDefaultFunc();

  const size_t MAX = 1e7;
  for (size_t idx = 0; idx < MAX; ++idx)
    mean += Eigen::Vector3f(1.f / (idx + 1), idx, 1.f);
  Eigen::Vector3f est(mean);

  EXPECT_LE((est - Eigen::Vector3f(0.00000166953f, 4999999.5f, 1.f)).norm(),
            1e-8);
}

TEST(gmMiscAccumulativeMean, EigenQuaternion) {

  gmMisc::AccumulativeMean<Eigen::Quaterniond> mean(
      Eigen::Quaterniond::Identity());
  mean.func_interp = [](double r,
                        const Eigen::Quaterniond &a,
                        const Eigen::Quaterniond &b) -> Eigen::Quaterniond {
    return a.slerp(r, b);
  };

  const size_t MAX = 1e6;
  for (size_t idx = 1; idx < MAX; ++idx)
    mean += Eigen::Quaterniond(
        Eigen::AngleAxisd(idx * GM_2_PI / MAX, Eigen::Vector3d::UnitZ()));

  Eigen::Quaterniond est(mean);
  Eigen::Quaterniond M(Eigen::AngleAxisd(GM_PI, Eigen::Vector3d::UnitZ()));
  EXPECT_LE(Eigen::AngleAxisd(est * M.conjugate()).angle(), 1e-8);
}

#endif
