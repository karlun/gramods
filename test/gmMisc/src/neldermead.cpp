
#include <gmMisc/config.hh>
#include <gmMisc/NelderMead.hh>

#include <Eigen/Eigen>

#ifdef gramods_ENABLE_Eigen3

using namespace gramods;

TEST(gmMiscNelderMead, Rosenbrock) {

  for (float a : {1.0f, 1.5f, 3.1f}) {
    float b = 100.0f;

    Eigen::Vector2f x0a(0, 0);
    Eigen::Vector2f x0b(1, 0);
    Eigen::Vector2f x0c(0, 1);
    std::vector<Eigen::Vector2f> x0({x0a, x0b, x0c});

    size_t iterations = 0;

    Eigen::Vector2f res = gmMisc::NelderMead::solve<float>(
        x0,
        [a, b](const Eigen::Vector2f &x) {
          return (a - x[0]) * (a - x[0]) +
                 b * (x[1] - x[0] * x[0]) * (x[1] - x[0] * x[0]);
        },
        iterations);

    EXPECT_NEAR(res[0], a, 1e-3);
    EXPECT_NEAR(res[1], a * a, 1e-3);
  }
}

TEST(gmMiscNelderMead, Booth) {

  Eigen::Vector2f x0a(0, 0);
  Eigen::Vector2f x0b(1, 0);
  Eigen::Vector2f x0c(0, 1);
  std::vector<Eigen::Vector2f> x0({x0a, x0b, x0c});

  size_t iterations = 0;

  Eigen::Vector2f res = gmMisc::NelderMead::solve<float>(
      x0,
      [](const Eigen::Vector2f &x) {
        return (x[0] + 2 * x[1] - 7) * (x[0] + 2 * x[1] - 7) +
               (2 * x[0] + x[1] - 5) * (2 * x[0] + x[1] - 5);
      },
      iterations);

  EXPECT_EQ(iterations, 72);
  EXPECT_NEAR(res[0], 1, 1e-3);
  EXPECT_NEAR(res[1], 3, 1e-3);
}

#endif

