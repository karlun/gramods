
#include <gmMisc/PolyFit.hh>

#ifdef gramods_ENABLE_Eigen3

using namespace gramods;

TEST(gmMiscPolyFit, Simple) {

  gmMisc::PolyFit poly(2, 3, 2);

#define X(A,B) (1 + (A) + (A)*(A)*(B))
#define Y(A,B) (-1 + (A) + (B))
#define Z(A,B) (2 - (A)*(A)*(B)*(B) + (A)*(B))

  for (double idx_b = 0; idx_b < 3.f; idx_b += 0.01f) {
    for (double idx_a = 0; idx_a < 3.f; idx_a += 0.01f) {
      double x = X(idx_a, idx_b);
      double y = Y(idx_a, idx_b);
      double z = Z(idx_a, idx_b);
      poly.addSample({idx_a, idx_b}, {x, y, z});
    }
  }

  auto polco = poly.estimateCoefficients();

  double max_err = 0.0;
  // Different increment for between-samples testing!
  for (double idx_b = 0; idx_b < 3.f; idx_b += 0.00667f) {
    for (double idx_a = 0; idx_a < 3.f; idx_a += 0.00667f) {

      auto res = poly.getValue({idx_a, idx_b});

      max_err = std::max(max_err, res[0] - X(idx_a, idx_b));
      max_err = std::max(max_err, res[1] - Y(idx_a, idx_b));
      max_err = std::max(max_err, res[2] - Z(idx_a, idx_b));
    }
  }

  EXPECT_LE(max_err, 1e-10);
}

TEST(gmMiscPolyFit, Derivative) {

  gmMisc::PolyFit poly(1, 1, 3);

  for (double idx = 0; idx < 3.f; idx += 0.01f)
    poly.addSample({idx}, {5 - 4 * idx + 3 * idx * idx - 2 * idx * idx * idx});

  auto polco = poly.estimateCoefficients();

  double max_err = 0.0;
  // Different increment for between-samples testing!
  for (double idx = 0; idx < 3.f; idx += 0.00667f) {

    auto res = poly.getDerivative(idx);
    auto der = -4 + 2 * 3 * idx - 3 * 2 * idx * idx;

    max_err = std::max(max_err, std::fabs(res - der));
  }

  EXPECT_LE(max_err, 1e-10);
}

TEST(gmMiscPolyFit, Jacobian) {

  gmMisc::PolyFit poly(2, 3, 2);

#define X(A,B) (1 + (A) + (A)*(A)*(B))
#define Y(A,B) (-1 + (A) + (B))
#define Z(A,B) (2 - (A)*(A)*(B)*(B) + (A)*(B))

#define dXdA(A,B) (1 + 2*(A)*(B))
#define dXdB(A,B) ((A)*(A))
#define dYdA(A,B) (1)
#define dYdB(A,B) (1)
#define dZdA(A,B) (- 2*(A)*(B)*(B) + (B))
#define dZdB(A,B) (- 2*(A)*(A)*(B) + (A))

  for (double idx_b = 0; idx_b < 3.f; idx_b += 0.01f) {
    for (double idx_a = 0; idx_a < 3.f; idx_a += 0.01f) {
      double x = X(idx_a, idx_b);
      double y = Y(idx_a, idx_b);
      double z = Z(idx_a, idx_b);
      poly.addSample({idx_a, idx_b}, {x, y, z});
    }
  }

  auto polco = poly.estimateCoefficients();

  double max_err = 0.0;
  // Different increment for between-samples testing!
  for (double idx_b = 0; idx_b < 3.f; idx_b += 0.00667f) {
    for (double idx_a = 0; idx_a < 3.f; idx_a += 0.00667f) {

      auto res = poly.getJacobian({idx_a, idx_b});
      ASSERT_EQ(res.cols(), 2);
      ASSERT_EQ(res.rows(), 3);

      max_err = std::max(max_err, std::fabs(res(0, 0) - dXdA(idx_a, idx_b)));
      max_err = std::max(max_err, std::fabs(res(1, 0) - dYdA(idx_a, idx_b)));
      max_err = std::max(max_err, std::fabs(res(2, 0) - dZdA(idx_a, idx_b)));
      max_err = std::max(max_err, std::fabs(res(0, 1) - dXdB(idx_a, idx_b)));
      max_err = std::max(max_err, std::fabs(res(1, 1) - dYdB(idx_a, idx_b)));
      max_err = std::max(max_err, std::fabs(res(2, 1) - dZdB(idx_a, idx_b)));
    }
  }
  EXPECT_LE(max_err, 1e-10);
}

#endif
