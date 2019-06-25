
#include <gmTypes/config.hh>

#ifdef gramods_ENABLE_Eigen3

#include <gmTypes/eigen.hh>

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

TEST(gmTypesEigen, XML_keyerror) {

  std::stringstream ss("quat 1 2 3 4");
  Eigen::Quaternionf q;
  ss >> q;

  EXPECT_EQ_EIGEN_QUAT(Eigen::Quaternionf::Identity(), q);
}

TEST(gmTypesEigen, XML_quaternion) {

  std::stringstream ss("quaternion 1 2 3 4");
  Eigen::Quaternionf q;
  ss >> q;

  Eigen::Quaternionf Q(1, 2, 3, 4);
  EXPECT_EQ_EIGEN_QUAT(Q, q);
}

TEST(gmTypesEigen, XML_EulerYPR) {

  Eigen::Quaternionf q;
  std::stringstream ss(R"lang=xml(
ypr
1.57079632679489661923
1.57079632679489661923
1.57079632679489661923)lang=xml");

  ss >> q;

  EXPECT_LE((q * Eigen::Vector3f::UnitX() + Eigen::Vector3f::UnitX()).norm(), 1e-5);
  EXPECT_LE((q * Eigen::Vector3f::UnitY() - Eigen::Vector3f::UnitZ()).norm(), 1e-5);
  EXPECT_LE((q * Eigen::Vector3f::UnitZ() - Eigen::Vector3f::UnitY()).norm(), 1e-5);
}

TEST(gmTypesEigen, XML_axisangle) {

  Eigen::Quaternionf q;
  std::stringstream ss(R"lang=xml(
axisangle 1 2 3 0.1
)lang=xml");

  ss >> q;

  Eigen::AngleAxisf A(0.1, Eigen::Vector3f(1, 2, 3).normalized());
  Eigen::Quaternionf Q(A);
  EXPECT_EQ_EIGEN_QUAT(Q, q);
}

TEST(gmTypesEigen, XML_angleaxis) {

  Eigen::Quaternionf q;
  std::stringstream ss(R"lang=xml(
angleaxis 0.1 1 2 3
)lang=xml");

  ss >> q;

  Eigen::AngleAxisf A(0.1, Eigen::Vector3f(1, 2, 3).normalized());
  Eigen::Quaternionf Q(A);
  EXPECT_EQ_EIGEN_QUAT(Q, q);
}

#endif
