
#include <gmTypes/config.hh>

#ifdef gramods_ENABLE_Eigen3

#include <gmCore/OFactory.hh>
#include <gmCore/ImportLibrary.hh>
#include <gmCore/Configuration.hh>

#include <gmTypes/eigen.hh>

#include <memory>
#include <string>
#include <vector>

#define EXPECT_EQ_EIGEN_QUAT(A,B)                       \
  EXPECT_GE(1e-10,                                      \
            ((A) * Eigen::Vector3f::UnitX() -           \
             (B) * Eigen::Vector3f::UnitX()).norm() +   \
            ((A) * Eigen::Vector3f::UnitY() -           \
             (B) * Eigen::Vector3f::UnitY()).norm() +   \
            ((A) * Eigen::Vector3f::UnitZ() -           \
             (B) * Eigen::Vector3f::UnitZ()).norm())

using namespace gramods;

struct Object : gmCore::Object {
  Eigen::Quaternionf q;
  Eigen::Vector3f v;
  void setQ(Eigen::Quaternionf _q) { q = _q; }
  void setV(Eigen::Vector3f _v) { v = _v; }
  GM_OFI_DECLARE;
};

GM_OFI_DEFINE(Object);
GM_OFI_PARAM(Object, q, Eigen::Quaternionf, Object::setQ);
GM_OFI_PARAM(Object, v, Eigen::Vector3f, Object::setV);

TEST(gmTypesEigen, XML_keyerror) {

  gmCore::Configuration config(R"lang=xml(
<config><Object q="quat 1 2 3 4"/></config>
)lang=xml" );

  std::shared_ptr<Object> test;
  EXPECT_TRUE(config.getObject(test));
  EXPECT_TRUE(test);

  EXPECT_EQ_EIGEN_QUAT(Eigen::Quaternionf::Identity(), test->q);
}

TEST(gmTypesEigen, XML_quaternion) {

  gmCore::Configuration config(R"lang=xml(
<config><Object q="quaternion 1 2 3 4"/></config>
)lang=xml" );

  std::shared_ptr<Object> test;
  EXPECT_TRUE(config.getObject(test));
  EXPECT_TRUE(test);

  Eigen::Quaternionf Q(1, 2, 3, 4);
  EXPECT_EQ_EIGEN_QUAT(Q, test->q);
}

TEST(gmTypesEigen, XML_EulerYPR) {

  gmCore::Configuration config(R"lang=xml(
<config><Object q="ypr
                   1.57079632679489661923
                   1.57079632679489661923
                   1.57079632679489661923"/></config>
)lang=xml" );

  std::shared_ptr<Object> test;
  EXPECT_TRUE(config.getObject(test));
  EXPECT_TRUE(test);

  EXPECT_LE((test->q * Eigen::Vector3f::UnitX() + Eigen::Vector3f::UnitX()).norm(), 1e-5);
  EXPECT_LE((test->q * Eigen::Vector3f::UnitY() - Eigen::Vector3f::UnitZ()).norm(), 1e-5);
  EXPECT_LE((test->q * Eigen::Vector3f::UnitZ() - Eigen::Vector3f::UnitY()).norm(), 1e-5);
}

TEST(gmTypesEigen, XML_axisangle) {

  gmCore::Configuration config(R"lang=xml(
<config><Object q="axisangle 1 2 3 0.1"/></config>
)lang=xml" );

  std::shared_ptr<Object> test;
  EXPECT_TRUE(config.getObject(test));
  EXPECT_TRUE(test);

  Eigen::AngleAxisf A(0.1, Eigen::Vector3f(1, 2, 3).normalized());
  Eigen::Quaternionf Q(A);
  EXPECT_EQ_EIGEN_QUAT(Q, test->q);
}

TEST(gmTypesEigen, XML_angleaxis) {

  gmCore::Configuration config(R"lang=xml(
<config><Object q="angleaxis 0.1 1 2 3"/></config>
)lang=xml" );

  std::shared_ptr<Object> test;
  EXPECT_TRUE(config.getObject(test));
  EXPECT_TRUE(test);

  Eigen::AngleAxisf A(0.1, Eigen::Vector3f(1, 2, 3).normalized());
  Eigen::Quaternionf Q(A);
  EXPECT_EQ_EIGEN_QUAT(Q, test->q);
}

#endif
