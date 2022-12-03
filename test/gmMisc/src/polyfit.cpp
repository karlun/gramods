
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

  double err = 0.0;
  size_t count = 0;
  // Different increment for between-samples testing!
  for (double idx_b = 0; idx_b < 3.f; idx_b += 0.00667f) {
    for (double idx_a = 0; idx_a < 3.f; idx_a += 0.00667f) {

      auto res = poly.getValue({idx_a, idx_b});

      double x = res[0] - X(idx_a, idx_b);
      double y = res[1] - Y(idx_a, idx_b);
      double z = res[2] - Z(idx_a, idx_b);
      err += x * x + y * y + z * z;

      ++count;
    }
  }

  err /= double(count);
  err = std::sqrt(err);
  EXPECT_LE(err, 1e-10); // Allowing for some numerical error
}

#endif
