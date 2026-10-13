
#include <gmCore/config.hh>

#ifdef gramods_ENABLE_Eigen3

#include <gmCore/io_angle.hh>
#include <gmCore/io_eigen.hh>
#include <gmCore/Pose.hh>

#include <sstream>

#define EXPECT_EQ_EIGEN_QUAT(A,B)                       \
  EXPECT_GE(1e-10,                                      \
            ((A) * Eigen::Vector3f::UnitX() -           \
             (B) * Eigen::Vector3f::UnitX()).norm() +   \
            ((A) * Eigen::Vector3f::UnitY() -           \
             (B) * Eigen::Vector3f::UnitY()).norm() +   \
            ((A) * Eigen::Vector3f::UnitZ() -           \
             (B) * Eigen::Vector3f::UnitZ()).norm())

using namespace gramods;

TEST(gmCoreEigen, XML_keyerror) {

  std::stringstream ss("quat 1 2 3 4");
  Eigen::Quaternionf q;
  ss >> q;

  EXPECT_EQ_EIGEN_QUAT(Eigen::Quaternionf::Identity(), q);
}

TEST(gmCoreEigen, XML_quaternion) {

  std::stringstream ss("quaternion 1 2 3 4");
  Eigen::Quaternionf q;
  ss >> q;

  Eigen::Quaternionf Q(1, 2, 3, 4);
  EXPECT_EQ_EIGEN_QUAT(Q, q);
}

TEST(gmCoreEigen, XML_EulerYPR) {

  Eigen::Quaternionf q;

  {
    std::stringstream ss(R"lang=xml(
ypr
1.57079632679489661923
0
0)lang=xml");

    ss >> q;

    EXPECT_LE((q * Eigen::Vector3f::UnitX() - Eigen::Vector3f::UnitZ()).norm(), 1e-5);
    EXPECT_LE((q * Eigen::Vector3f::UnitY() - Eigen::Vector3f::UnitY()).norm(), 1e-5);
    EXPECT_LE((q * Eigen::Vector3f::UnitZ() + Eigen::Vector3f::UnitX()).norm(), 1e-5);
  }
  {
    std::stringstream ss(R"lang=xml(
ypr
0
1.57079632679489661923
0)lang=xml");

    ss >> q;

    EXPECT_LE((q * Eigen::Vector3f::UnitX() - Eigen::Vector3f::UnitX()).norm(), 1e-5);
    EXPECT_LE((q * Eigen::Vector3f::UnitY() - Eigen::Vector3f::UnitZ()).norm(), 1e-5);
    EXPECT_LE((q * Eigen::Vector3f::UnitZ() + Eigen::Vector3f::UnitY()).norm(), 1e-5);
  }
  {
    std::stringstream ss(R"lang=xml(
ypr
3.14159265358979323846
1.57079632679489661923
1.57079632679489661923)lang=xml");

    ss >> q;

    EXPECT_LE((q * Eigen::Vector3f::UnitX() - Eigen::Vector3f::UnitZ()).norm(), 1e-5);
    EXPECT_LE((q * Eigen::Vector3f::UnitY() + Eigen::Vector3f::UnitX()).norm(), 1e-5);
    EXPECT_LE((q * Eigen::Vector3f::UnitZ() + Eigen::Vector3f::UnitY()).norm(), 1e-5);
  }
}

TEST(gmCoreEigen, XML_axisangle_rad) {

  Eigen::Quaternionf q;
  std::stringstream ss(R"lang=xml(
axisangle 1 2 3 0.1
)lang=xml");

  ss >> q;

  Eigen::AngleAxisf A(0.1f, Eigen::Vector3f(1, 2, 3).normalized());
  Eigen::Quaternionf Q(A);
  EXPECT_EQ_EIGEN_QUAT(Q, q);
}

TEST(gmCoreEigen, XML_angleaxis_rad) {

  Eigen::Quaternionf q;
  std::stringstream ss(R"lang=xml(
angleaxis 0.1 1 2 3
)lang=xml");

  ss >> q;

  Eigen::AngleAxisf A(0.1, Eigen::Vector3f(1, 2, 3).normalized());
  Eigen::Quaternionf Q(A);
  EXPECT_EQ_EIGEN_QUAT(Q, q);
}

TEST(gmCoreEigen, XML_axisangle_deg) {

  Eigen::Quaternionf q;
  std::stringstream ss(R"lang=xml(
axisangle 1 2 3 d10
)lang=xml");

  ss >> q;

  Eigen::AngleAxisf A(gmCore::angle::from_degrees * 10.f,
                      Eigen::Vector3f(1, 2, 3).normalized());
  Eigen::Quaternionf Q(A);
  EXPECT_EQ_EIGEN_QUAT(Q, q);
}

TEST(gmCoreEigen, XML_angleaxis_deg) {

  Eigen::Quaternionf q;
  std::stringstream ss(R"lang=xml(
angleaxis d10 1 2 3
)lang=xml");

  ss >> q;

  Eigen::AngleAxisf A(gmCore::angle::from_degrees * 10.f,
                      Eigen::Vector3f(1, 2, 3).normalized());
  Eigen::Quaternionf Q(A);
  EXPECT_EQ_EIGEN_QUAT(Q, q);
}

TEST(gmCoreEigen, Pose) {
  const auto A =
      gmCore::Pose{.position = {7.5, -1.2, -2.0},
                   .orientation = Eigen::Quaternionf(
                       Eigen::AngleAxisf(0.3, Eigen::Vector3f::UnitZ()))};
  const auto B =
      gmCore::Pose{.position = {-0.5, 0.1, 0.99},
                   .orientation = Eigen::Quaternionf(
                       Eigen::AngleAxisf(0.5, Eigen::Vector3f::UnitX()))};

  const Eigen::Matrix4f M_C_P = (A * B).asMatrix().matrix();
  const Eigen::Matrix4f M_C_M = (A.asMatrix() * B.asMatrix()).matrix();
  EXPECT_LE(std::abs((M_C_P - M_C_M).sum()), 1e-5);

  const Eigen::Matrix4f M_D_P = A.conjugate().asMatrix().matrix();
  const Eigen::Matrix4f M_D_M = A.asMatrix().inverse().matrix();
  EXPECT_LE(std::abs((M_D_P - M_D_M).sum()), 1e-5);

  const Eigen::Matrix4f M_E_P = (A.conjugate() * B).asMatrix().matrix();
  const Eigen::Matrix4f M_E_M = (A.asMatrix().inverse() * B.asMatrix()).matrix();
  EXPECT_LE(std::abs((M_E_P - M_E_M).sum()), 1e-5);

  const auto D =
      gmCore::Pose{.position = {4.52, -0.1, 3.87},
                   .orientation = Eigen::Quaternionf(
                       Eigen::AngleAxisf(-2.5, Eigen::Vector3f::UnitY()))};
  const auto A2 = D * A;
  const auto B2 = D * B;

  const gmCore::Pose PD_1 = A.conjugate() * B - A2.conjugate() * B2;
  EXPECT_LE(PD_1.position.norm(), 1e-5);
  EXPECT_LE(std::abs(1 - PD_1.orientation.w()), 1e-5);
}

#endif
