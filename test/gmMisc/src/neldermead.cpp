
#include <gmMisc/NelderMead.hh>

#ifdef gramods_ENABLE_Eigen3

#include <Eigen/Eigen>
#include <random>

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

  EXPECT_EQ(iterations, 57);
  EXPECT_NEAR(res[0], 1, 1e-3);
  EXPECT_NEAR(res[1], 3, 1e-3);
}

#  define ERR1(X) ((X)*real_random(random_engine))
#  define ERR3(X)                                                              \
    (X) * Eigen::Vector3f(real_random(random_engine),                          \
                          real_random(random_engine),                          \
                          real_random(random_engine))

TEST(gmMiscNelderMead, Multilateration4D) {

  std::default_random_engine random_engine;
  std::uniform_real_distribution<float> real_random =
    std::uniform_real_distribution<float>(-1.f, 1.f);

  std::vector<Eigen::Vector3f> mic_pts = {Eigen::Vector3f(-1, -1, -1),
                                          Eigen::Vector3f(+1, +1, -1),
                                          Eigen::Vector3f(-1, +1, +1),
                                          Eigen::Vector3f(+1, -1, +1)};
  Eigen::Vector3f mic_mid = Eigen::Vector3f::Zero();
  for (const auto &pt : mic_pts) mic_mid += pt;
  mic_mid /= mic_pts.size();

  std::vector<Eigen::Vector3f> mic_pts_err;
  for (const auto pt : mic_pts) {
    mic_pts_err.push_back(pt + ERR3(0.01));
  }

  float mic_R = 0.f;
  for (const auto &pt : mic_pts) mic_R = std::max(mic_R, (pt - mic_mid).norm());

  std::vector<Eigen::Vector3f> test_points = {{0.3, -0.2, 0.6},
                                              {-0.9, 0.1, 0.1},
                                              {0.0, 0.0, 0.0},
                                              {0.715, 0.736, -0.791},
                                              {5.0, 5.0, 0.4},
                                              {1.5, 0.2, 0.4}};

  for (size_t idx = 0; idx < 1000; ++idx)
    test_points.push_back({0.9f * real_random(random_engine),
                           0.9f * real_random(random_engine),
                           0.9f * real_random(random_engine)});

  float error_sum = 0;
  for (auto p : test_points) {

    // Set up solution simplex
    std::vector<Eigen::Vector4f> x0;
    for (const auto pt : mic_pts) {
      Eigen::Vector4f h_pt;
      h_pt << pt, 0.6f;
      x0.push_back(h_pt);
    }
    {
      Eigen::Vector4f h_pt;
      h_pt << mic_mid, 0.3f;
      x0.push_back(h_pt);
    }

    // The actual distances
    Eigen::Vector4f d_toa((mic_pts[0] - p).norm() + ERR1(0.001),
                          (mic_pts[1] - p).norm() + ERR1(0.001),
                          (mic_pts[2] - p).norm() + ERR1(0.001),
                          (mic_pts[3] - p).norm() + ERR1(0.001));
    // What is known about time-of-arrival, expressed in distance since velocity is known:
    d_toa.array() -= d_toa.minCoeff();

    size_t iterations = 0;
    Eigen::Vector4f res4 = gmMisc::NelderMead::solve<float>(
        x0,
        [&mic_pts_err, &d_toa](const Eigen::Vector4f &x) {
          Eigen::Vector3f p = x.block<3, 1>(0, 0);
          Eigen::Vector4f d((mic_pts_err[0] - p).norm(),
                            (mic_pts_err[1] - p).norm(),
                            (mic_pts_err[2] - p).norm(),
                            (mic_pts_err[3] - p).norm());
          d.array() -= x[3]; // Instead of d_toa + d0
          return (d - d_toa).squaredNorm();
        },
        iterations);

    Eigen::Vector3f res = res4.block<3, 1>(0, 0);

    float err = (res - p).squaredNorm();
    EXPECT_LE(err, 1e-3) << "Square error for pt " << p.transpose();
    error_sum += err;
  }
  EXPECT_LE(error_sum / test_points.size(), 1e-3)
      << "Mean square error for all points";
}

TEST(gmMiscNelderMead, MultilaterationSpacetimeCone) {

  std::default_random_engine random_engine;
  std::uniform_real_distribution<float> real_random =
      std::uniform_real_distribution<float>(-1.f, 1.f);

  std::vector<Eigen::Vector3f> mic_pts = {Eigen::Vector3f(-1, -1, -1),
                                          Eigen::Vector3f(+1, +1, -1),
                                          Eigen::Vector3f(-1, +1, +1),
                                          Eigen::Vector3f(+1, -1, +1)};
  Eigen::Vector3f mic_mid = Eigen::Vector3f::Zero();
  for (const auto &pt : mic_pts) mic_mid += pt;
  mic_mid /= mic_pts.size();

  float mic_R = 0.f;
  for (const auto &pt : mic_pts) mic_R = std::max(mic_R, (pt - mic_mid).norm());

  std::vector<Eigen::Vector3f> test_points = {{0.3, -0.2, 0.6},
                                              {-0.9, 0.1, 0.1},
                                              {0.0, 0.0, 0.0},
                                              {0.715, 0.736, -0.791},
                                              {5.0, 5.0, 0.4},
                                              {1.5, 0.2, 0.4}};

  for (size_t idx = 0; idx < 1000; ++idx)
    test_points.push_back({0.9f * real_random(random_engine),
                           0.9f * real_random(random_engine),
                           0.9f * real_random(random_engine)});

  float error_sum = 0;
  for (auto p : test_points) {

    // Actually unknown microphone distances (sound speed = 1)
    std::vector<float> mic_dists;
    for (const auto &pt : mic_pts)
      mic_dists.push_back((pt - p).norm());

    float dmin = std::numeric_limits<float>::max();
    for (const auto &dist : mic_dists)
      dmin = std::min(dmin, dist);

    // Known _relative_ spacetime microphone positions
    std::vector<Eigen::Vector4f> h_mic_pts;
    for (size_t idx = 0; idx < mic_pts.size(); ++idx) {
      Eigen::Vector4f h_pt;
      h_pt << mic_pts[idx] + ERR3(0.01), mic_dists[idx] - dmin + ERR1(0.001);
      h_mic_pts.push_back(h_pt);
    }

    // Set up solution simplex
    std::vector<Eigen::Vector4f> x0;
    for (size_t idx = 0; idx < 4; ++idx) {
      Eigen::Vector4f h_pt;
      h_pt << mic_mid + 0.4f * (mic_pts[idx] - mic_mid), -1.9f * mic_R;
      x0.push_back(h_pt);
    }
    {
      Eigen::Vector4f h_pt;
      h_pt << mic_mid, -mic_R;
      x0.push_back(h_pt);
    }

    size_t iterations = 0;
    Eigen::Vector4f res4 = gmMisc::NelderMead::solve<float>(
        x0,
        [&h_mic_pts, mic_R](const Eigen::Vector4f &X) {

          // Sound cannot have been emitted in the future
          if (X[3] >= 0)
            return std::numeric_limits<float>::max();

          float err2 = 0;
          for (const auto &pt : h_mic_pts) {

            const Eigen::Vector4f p4 = pt - X;
            const Eigen::Vector3f p3 = p4.block<3, 1>(0, 0);
            const float L3 = p3.norm();

            // Sound cannot have been emitted at a microphone
            if (L3 < std::numeric_limits<float>::epsilon() * mic_R)
              return std::numeric_limits<float>::max();

            const Eigen::Vector3f n3 = p3.normalized();
            Eigen::Vector4f n4; n4 << n3, -1;
            n4.normalize();

            const Eigen::Vector4f pp =
                p4[3] * Eigen::Vector4f(n3[0], n3[1], n3[2], 1);
            const Eigen::Vector4f pc = p4 + n4 * (pp - p4).dot(n4);

            err2 += (p4 - pc).squaredNorm();
          }

          return err2;
        },
        iterations);

    Eigen::Vector3f res = res4.block<3, 1>(0, 0);

    float err = (res - p).squaredNorm();
    EXPECT_LE(err, 4e-3) << "Square error for pt " << p.transpose();
    error_sum += err;
  }
  EXPECT_LE(error_sum / test_points.size(), 1e-3)
      << "Mean square error for all points";
}

#endif

