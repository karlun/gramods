
#include <gmTrack/RegisteredSinglePoseTracker.hh>
#include <gmTrack/RegisteredMultiPoseTracker.hh>
#include <gmTrack/SingleToMultiPoseTracker.hh>
#include <gmTrack/TimeSamplePoseTracker.hh>

#include <gmCore/Updateable.hh>

#include <gmCore/Console.hh>
#include <gmCore/OStreamMessageSink.hh>
#include <gmCore/MathConstants.hh>
#include <gmCore/NullMessageSink.hh>

using namespace gramods;

#define EXPECT_EQ_EIGEN_VECTOR(A, B)                                           \
  EXPECT_GE(1e-10, ((A) - (B)).squaredNorm());
#define EXPECT_EQ_EIGEN_ORIENT(A, B)                                           \
  EXPECT_GE(                                                                   \
      1e-5,                                                                    \
      ((A)*Eigen::Vector3f::UnitX() - (B)*Eigen::Vector3f::UnitX()).norm() +   \
          ((A)*Eigen::Vector3f::UnitY() - (B)*Eigen::Vector3f::UnitY())        \
              .norm() +                                                        \
          ((A)*Eigen::Vector3f::UnitZ() - (B)*Eigen::Vector3f::UnitZ())        \
              .norm())

TEST(gmTrackRegisteredPose, SinglePose) {

  gmCore::Console::removeAllSinks();
#if 0
  std::shared_ptr<gmCore::OStreamMessageSink> osms =
    std::make_shared<gmCore::OStreamMessageSink>();
  osms->initialize();
#else
  std::shared_ptr<gmCore::NullMessageSink> nullsink =
    std::make_shared<gmCore::NullMessageSink>();
  nullsink->initialize();
#endif

  auto pose_tracker = std::make_shared<gmTrack::TimeSamplePoseTracker>();
  pose_tracker->addPosition(Eigen::Vector3f(0, 0, 0));
  pose_tracker->addOrientation(
      Eigen::Quaternionf(Eigen::AngleAxisf(0, Eigen::Vector3f::UnitX())));
  pose_tracker->addPosition(Eigen::Vector3f(3, 0, 0));
  pose_tracker->addOrientation(
      Eigen::Quaternionf(Eigen::AngleAxisf(GM_PI_2, Eigen::Vector3f::UnitX())));
  pose_tracker->addPosition(Eigen::Vector3f(0, 3, 0));
  pose_tracker->addOrientation(
      Eigen::Quaternionf(Eigen::AngleAxisf(GM_PI_2, Eigen::Vector3f::UnitX())));
  pose_tracker->initialize();

  auto reg_tracker = std::make_shared<gmTrack::RegisteredSinglePoseTracker>();
  reg_tracker->setSinglePoseTracker(pose_tracker);
  reg_tracker->initialize();

  gmTrack::PoseTracker::PoseSample pose;

  EXPECT_TRUE(reg_tracker->getPose(pose));
  EXPECT_EQ_EIGEN_VECTOR(pose.position, Eigen::Vector3f(0, 0, 0));
  EXPECT_EQ_EIGEN_ORIENT(pose.orientation,
                         Eigen::AngleAxisf(0, Eigen::Vector3f::UnitX()));
  EXPECT_TRUE(reg_tracker->getPose(pose));
  EXPECT_EQ_EIGEN_VECTOR(pose.position, Eigen::Vector3f(3, 0, 0));
  EXPECT_EQ_EIGEN_ORIENT(pose.orientation,
                         Eigen::AngleAxisf(GM_PI_2, Eigen::Vector3f::UnitX()));
  EXPECT_TRUE(reg_tracker->getPose(pose));
  EXPECT_EQ_EIGEN_VECTOR(pose.position, Eigen::Vector3f(0, 3, 0));
  EXPECT_EQ_EIGEN_ORIENT(pose.orientation,
                         Eigen::AngleAxisf(GM_PI_2, Eigen::Vector3f::UnitX()));

  reg_tracker->setPositionBias(Eigen::Vector3f(0, 1, 2));

  EXPECT_TRUE(reg_tracker->getPose(pose));
  EXPECT_EQ_EIGEN_VECTOR(pose.position, Eigen::Vector3f(0, 1, 2));
  EXPECT_EQ_EIGEN_ORIENT(pose.orientation,
                         Eigen::AngleAxisf(0, Eigen::Vector3f::UnitX()));
  EXPECT_TRUE(reg_tracker->getPose(pose));
  EXPECT_EQ_EIGEN_VECTOR(pose.position, Eigen::Vector3f(3, -2, 1));
  EXPECT_EQ_EIGEN_ORIENT(pose.orientation,
                         Eigen::AngleAxisf(GM_PI_2, Eigen::Vector3f::UnitX()));
  EXPECT_TRUE(reg_tracker->getPose(pose));
  EXPECT_EQ_EIGEN_VECTOR(pose.position, Eigen::Vector3f(0, 1, 1));
  EXPECT_EQ_EIGEN_ORIENT(pose.orientation,
                         Eigen::AngleAxisf(GM_PI_2, Eigen::Vector3f::UnitX()));

  reg_tracker->setPositionBias(Eigen::Vector3f(1, 2, 3));
  reg_tracker->setOrientationBias(
      Eigen::Quaternionf(Eigen::AngleAxisf(GM_PI_2, Eigen::Vector3f::UnitX())));

  EXPECT_TRUE(reg_tracker->getPose(pose));
  EXPECT_EQ_EIGEN_VECTOR(pose.position, Eigen::Vector3f(1, 2, 3));
  EXPECT_EQ_EIGEN_ORIENT(pose.orientation,
                         Eigen::AngleAxisf(GM_PI_2, Eigen::Vector3f::UnitX()));
  EXPECT_TRUE(reg_tracker->getPose(pose));
  EXPECT_EQ_EIGEN_VECTOR(pose.position, Eigen::Vector3f(4, -3, 2));
  EXPECT_EQ_EIGEN_ORIENT(pose.orientation,
                         Eigen::AngleAxisf(GM_PI, Eigen::Vector3f::UnitX()));
  EXPECT_TRUE(reg_tracker->getPose(pose));
  EXPECT_EQ_EIGEN_VECTOR(pose.position, Eigen::Vector3f(1, 0, 2));
  EXPECT_EQ_EIGEN_ORIENT(pose.orientation,
                         Eigen::AngleAxisf(GM_PI, Eigen::Vector3f::UnitX()));
}

TEST(gmTrackRegisteredPose, MultiPose) {

  gmCore::Console::removeAllSinks();
#if 0
  std::shared_ptr<gmCore::OStreamMessageSink> osms =
    std::make_shared<gmCore::OStreamMessageSink>();
  osms->initialize();
#else
  std::shared_ptr<gmCore::NullMessageSink> nullsink =
      std::make_shared<gmCore::NullMessageSink>();
  nullsink->initialize();
#endif

  auto pose_tracker = std::make_shared<gmTrack::TimeSamplePoseTracker>();
  pose_tracker->addPosition(Eigen::Vector3f(0, 0, 0));
  pose_tracker->addOrientation(
      Eigen::Quaternionf(Eigen::AngleAxisf(0, Eigen::Vector3f::UnitX())));
  pose_tracker->addPosition(Eigen::Vector3f(3, 0, 0));
  pose_tracker->addOrientation(
      Eigen::Quaternionf(Eigen::AngleAxisf(GM_PI_2, Eigen::Vector3f::UnitX())));
  pose_tracker->addPosition(Eigen::Vector3f(0, 3, 0));
  pose_tracker->addOrientation(
      Eigen::Quaternionf(Eigen::AngleAxisf(GM_PI_2, Eigen::Vector3f::UnitX())));
  pose_tracker->initialize();

  auto multi_tracker = std::make_shared<gmTrack::SingleToMultiPoseTracker>();
  multi_tracker->addSinglePoseTracker(pose_tracker);
  multi_tracker->initialize();

  auto reg_tracker = std::make_shared<gmTrack::RegisteredMultiPoseTracker>();
  reg_tracker->setMultiPoseTracker(multi_tracker);
  reg_tracker->initialize();

  std::map<int, gmTrack::PoseTracker::PoseSample> poses;
  gmTrack::PoseTracker::PoseSample pose;

#define GET_POSE                                                               \
  EXPECT_TRUE(reg_tracker->getPose(poses));                                    \
  ASSERT_EQ(poses.size(), 1);                                                  \
  pose = poses.begin()->second;

  GET_POSE;
  EXPECT_EQ_EIGEN_VECTOR(pose.position, Eigen::Vector3f(0, 0, 0));
  EXPECT_EQ_EIGEN_ORIENT(pose.orientation,
                         Eigen::AngleAxisf(0, Eigen::Vector3f::UnitX()));
  GET_POSE;
  EXPECT_EQ_EIGEN_VECTOR(pose.position, Eigen::Vector3f(3, 0, 0));
  EXPECT_EQ_EIGEN_ORIENT(pose.orientation,
                         Eigen::AngleAxisf(GM_PI_2, Eigen::Vector3f::UnitX()));
  GET_POSE;
  EXPECT_EQ_EIGEN_VECTOR(pose.position, Eigen::Vector3f(0, 3, 0));
  EXPECT_EQ_EIGEN_ORIENT(pose.orientation,
                         Eigen::AngleAxisf(GM_PI_2, Eigen::Vector3f::UnitX()));

  reg_tracker->setPositionBias(Eigen::Vector3f(0, 1, 2));

  GET_POSE;
  EXPECT_EQ_EIGEN_VECTOR(pose.position, Eigen::Vector3f(0, 1, 2));
  EXPECT_EQ_EIGEN_ORIENT(pose.orientation,
                         Eigen::AngleAxisf(0, Eigen::Vector3f::UnitX()));
  GET_POSE;
  EXPECT_EQ_EIGEN_VECTOR(pose.position, Eigen::Vector3f(3, -2, 1));
  EXPECT_EQ_EIGEN_ORIENT(pose.orientation,
                         Eigen::AngleAxisf(GM_PI_2, Eigen::Vector3f::UnitX()));
  GET_POSE;
  EXPECT_EQ_EIGEN_VECTOR(pose.position, Eigen::Vector3f(0, 1, 1));
  EXPECT_EQ_EIGEN_ORIENT(pose.orientation,
                         Eigen::AngleAxisf(GM_PI_2, Eigen::Vector3f::UnitX()));

  reg_tracker->setPositionBias(Eigen::Vector3f(1, 2, 3));
  reg_tracker->setOrientationBias(
      Eigen::Quaternionf(Eigen::AngleAxisf(GM_PI_2, Eigen::Vector3f::UnitX())));

  GET_POSE;
  EXPECT_EQ_EIGEN_VECTOR(pose.position, Eigen::Vector3f(1, 2, 3));
  EXPECT_EQ_EIGEN_ORIENT(pose.orientation,
                         Eigen::AngleAxisf(GM_PI_2, Eigen::Vector3f::UnitX()));
  GET_POSE;
  EXPECT_EQ_EIGEN_VECTOR(pose.position, Eigen::Vector3f(4, -3, 2));
  EXPECT_EQ_EIGEN_ORIENT(pose.orientation,
                         Eigen::AngleAxisf(GM_PI, Eigen::Vector3f::UnitX()));
  GET_POSE;
  EXPECT_EQ_EIGEN_VECTOR(pose.position, Eigen::Vector3f(1, 0, 2));
  EXPECT_EQ_EIGEN_ORIENT(pose.orientation,
                         Eigen::AngleAxisf(GM_PI, Eigen::Vector3f::UnitX()));
}
