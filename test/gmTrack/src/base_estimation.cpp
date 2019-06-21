
#include <gmTrack/PoseRegistrationEstimator.hh>

#include <gmTrack/TimeSamplePoseTracker.hh>

#include <gmCore/Updateable.hh>

#include <gmCore/Console.hh>
#include <gmCore/OStreamMessageSink.hh>
#include <gmCore/Configuration.hh>

using namespace gramods;

TEST(gmTrackBaseEstimation, FullSamplesByInverse) {

  gmCore::Console::removeAllSinks();
#if 0
  std::shared_ptr<gmCore::OStreamMessageSink> osms =
    std::make_shared<gmCore::OStreamMessageSink>();
  osms->initialize();
#endif

  {
    std::string xml = R"lang=xml(
<config>

  <ImportLibrary library="libgmTrack.so"/>

  <TimeSamplePoseTracker DEF="pose_tracker"/>

  <PoseRegistrationEstimator>

    <Controller>

      <TimeSamplePoseTracker USE="pose_tracker"/>

      <TimeSampleButtonsTracker>
        <param name="buttons" value="0"/>
        <param name="buttons" value="1"/>
        <param name="buttons" value="0"/>
        <param name="buttons" value="1"/>
        <param name="buttons" value="0"/>
        <param name="buttons" value="1"/>
        <param name="buttons" value="0"/>
        <param name="buttons" value="1"/>
        <param name="buttons" value="0"/>
      </TimeSampleButtonsTracker>
    </Controller>
    
  </PoseRegistrationEstimator>
</config>
)lang=xml";
    gmCore::Configuration config(xml);

    std::shared_ptr<gmTrack::PoseRegistrationEstimator> registrator;
    ASSERT_TRUE(config.getObject(registrator));

    std::shared_ptr<gmTrack::TimeSamplePoseTracker> tracker;
    ASSERT_TRUE(config.getObject(tracker));

    std::vector<std::shared_ptr<gmCore::Object>> objects;
    config.getAllObjects(objects);

    Eigen::Matrix4f RegA;
    RegA <<
      0, 0.5, 0, 1,
      -0.5, 0, 0, 2,
      0, 0, 0.5, 3,
      0, 0, 0, 1;

    Eigen::Matrix4f RegB;
    RegB <<
      0,   1,   0, 0.5,
      -1,   0,   0, 2.5,
      0,   0,   1, 4.5,
      0,   0,   0,   1;

    Eigen::Matrix4f points;
    points <<
      3, 0, 0, 3,
      0, 3, 0, 3,
      0, 0, 3, 3,
      1, 1, 1, 1;

    Eigen::Matrix4f samples = RegA.inverse() * points;

    for (int idx = 0; idx < 4; ++idx)
      registrator->addPoint({points(0, idx),
                             points(1, idx),
                             points(2, idx)});
    for (int idx = 0; idx < 4; ++idx)
      tracker->addPosition({samples(0, idx),
                            samples(1, idx),
                            samples(2, idx)});

    for (int idx = 1; idx <= 9; ++idx) {
      gmCore::Updateable::updateAll();
    }

    Eigen::Matrix4f Raw;
    Eigen::Matrix4f Unit;

    EXPECT_TRUE(registrator->getRegistration(&Raw, &Unit));

    EXPECT_LT((Raw - RegA).norm(), 1e-5);
    EXPECT_LT((Unit - RegB).norm(), 1e-5);
  }
}

TEST(gmTrackBaseEstimation, OverDeterminedSamplesByQR) {

  gmCore::Console::removeAllSinks();
#if 0
  std::shared_ptr<gmCore::OStreamMessageSink> osms =
    std::make_shared<gmCore::OStreamMessageSink>();
  osms->initialize();
#endif

  {
    std::string xml = R"lang=xml(
<config>

  <ImportLibrary library="libgmTrack.so"/>

  <TimeSamplePoseTracker DEF="pose_tracker"/>

  <PoseRegistrationEstimator>

    <Controller>

      <TimeSamplePoseTracker USE="pose_tracker"/>

      <TimeSampleButtonsTracker>
        <param name="buttons" value="0"/>
        <param name="buttons" value="1"/>
        <param name="buttons" value="0"/>
        <param name="buttons" value="1"/>
        <param name="buttons" value="0"/>
        <param name="buttons" value="1"/>
        <param name="buttons" value="0"/>
        <param name="buttons" value="1"/>
        <param name="buttons" value="0"/>
      </TimeSampleButtonsTracker>
    </Controller>
    
  </PoseRegistrationEstimator>
</config>
)lang=xml";
    gmCore::Configuration config(xml);

    std::shared_ptr<gmTrack::PoseRegistrationEstimator> registrator;
    ASSERT_TRUE(config.getObject(registrator));

    std::shared_ptr<gmTrack::TimeSamplePoseTracker> tracker;
    ASSERT_TRUE(config.getObject(tracker));

    std::vector<std::shared_ptr<gmCore::Object>> objects;
    config.getAllObjects(objects);

    Eigen::Matrix4f RegA;
    RegA <<
      0, 0.5, 0, 1,
      -0.5, 0, 0, 2,
      0, 0, 0.5, 1.5,
      0, 0, 0, 1;

    Eigen::Matrix4f RegB;
    RegB <<
      0,   1,   0,   0,
      -1,   0,   0, 2.25,
      0,   0,   1,   0,
      0,   0,   0,   1;

    Eigen::Matrix4f points;
    points <<
      3, 0, 2, 3,
      0, 3, 1, 3,
      3, 3, 3, 3,
      1, 1, 1, 1;

    Eigen::Matrix4f samples = RegA.inverse() * points;

    for (int idx = 0; idx < 4; ++idx)
      registrator->addPoint({points(0, idx),
                             points(1, idx),
                             points(2, idx)});
    for (int idx = 0; idx < 4; ++idx)
      tracker->addPosition({samples(0, idx),
                            samples(1, idx),
                            samples(2, idx)});

    for (int idx = 1; idx <= 9; ++idx) {
      gmCore::Updateable::updateAll();
    }

    Eigen::Matrix4f Raw;
    Eigen::Matrix4f Unit;

    EXPECT_TRUE(registrator->getRegistration(&Raw, &Unit));

    EXPECT_LT((Raw - RegA).norm(), 1e-5);
    EXPECT_LT((Unit - RegB).norm(), 1e-5);
  }
}
