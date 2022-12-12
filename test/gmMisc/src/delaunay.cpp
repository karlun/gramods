
#include <gmMisc/DelaunayInterpolator.hh>

#if defined(gramods_ENABLE_Eigen3) && defined(gramods_ENABLE_Lehdari_Delaunay)

#include <cmath>

using namespace gramods;

TEST(gmMiscDelaunay, Interpolation) {

  gmMisc::DelaunayInterpolator dip(2, 2);

  /*
    Simply triangle for testing interpolation:
    .______.
    |\    /|
    | \  / |
    |  \/  |
    |  /\  |
    | /  \ | <-- Interpolate here
    |/____\|
  */

  dip.addSample({ 0.0,  0.0}, {1.0, 1.0});
  dip.addSample({10.0,  0.0}, {2.0, 1.0});
  dip.addSample({ 0.0, 10.0}, {1.0, 2.0});
  dip.addSample({10.0, 10.0}, {2.0, 2.0});
  dip.addSample({ 5.0,  5.0}, {1.5, 1.5});

  EXPECT_EQ(dip.getValue({1.0, 10.1}).size(), 0);
 
  for (size_t idx = 0; idx < 100; ++idx) {

    std::vector<double> in({idx / double(10), 3.0});
    auto out = dip.getValue(in);

    // Allowing for some numerical error
    EXPECT_LE(fabs(out[0] - (0.1 * in[0] + 1.0)), 1e-10);
    EXPECT_LE(fabs(out[1] - (0.1 * in[1] + 1.0)), 1e-10);
  }
}

#endif
