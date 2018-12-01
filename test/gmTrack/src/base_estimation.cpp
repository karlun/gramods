
#include <gmTrack/BaseEstimator.hh>

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
    std::string xml = R"XML(
<config>

  <ImportLibrary lib="libgmTrack.so"/>

  <BaseEstimator>

    <param name="point" value=" 3  0  0"/>
    <param name="point" value=" 0  3  0"/>
    <param name="point" value=" 0  0  3"/>
    <param name="point" value=" 3  3  3"/>

    <Controller>

      <TimeSamplePoseTracker>
        <param name="position" value="-4  4 -6"/>
        <param name="position" value=" 2 -2 -6"/>
        <param name="position" value="-4 -2  0"/>
        <param name="position" value=" 2  4  0"/>
      </TimeSamplePoseTracker>

      <TimeSampleButtonsTracker>
        <param name="buttonStates" value="0"/>
        <param name="buttonStates" value="1"/>
        <param name="buttonStates" value="0"/>
        <param name="buttonStates" value="1"/>
        <param name="buttonStates" value="0"/>
        <param name="buttonStates" value="1"/>
        <param name="buttonStates" value="0"/>
        <param name="buttonStates" value="1"/>
        <param name="buttonStates" value="0"/>
      </TimeSampleButtonsTracker>
    </Controller>
    
  </BaseEstimator>
</config>
)XML";
    gmCore::Configuration config(xml);

    std::shared_ptr<gmTrack::BaseEstimator> tracker;
    config.getObject(tracker);

    std::vector<std::shared_ptr<gmCore::Object>> objects;
    config.getAllObjects(objects);

    for (int idx = 1; idx <= 9; ++idx) {
      gmCore::Updateable::updateAll();
    }

    Eigen::Matrix4f Raw;
    Eigen::Matrix4f Unit;

    EXPECT_TRUE(tracker->getRegistration(&Raw, &Unit));

    Eigen::Matrix4f RegA;
    RegA <<
      0, 0.5, 0, 1,
      0.5, 0, 0, 2,
      0, 0, 0.5, 3,
      0, 0, 0, 1;

    Eigen::Matrix4f RegB;
    RegB <<
      0,   1,   0, 0.5,
      1,   0,   0, 2.5,
      0,   0,   1, 4.5,
      0,   0,   0,   1;

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
    std::string xml = R"XML(
<config>

  <ImportLibrary lib="libgmTrack.so"/>

  <BaseEstimator>

    <param name="point" value=" 3  0  3"/>
    <param name="point" value=" 0  3  3"/>
    <param name="point" value=" 2  1  3"/>
    <param name="point" value=" 3  3  3"/>

    <Controller>

      <TimeSamplePoseTracker>
        <param name="position" value="-4  4  3  1"/>
        <param name="position" value=" 2 -2  3  1"/>
        <param name="position" value="-2  2  3  1"/>
        <param name="position" value=" 2  4  3  1"/>
      </TimeSamplePoseTracker>

      <TimeSampleButtonsTracker>
        <param name="buttonStates" value="0"/>
        <param name="buttonStates" value="1"/>
        <param name="buttonStates" value="0"/>
        <param name="buttonStates" value="1"/>
        <param name="buttonStates" value="0"/>
        <param name="buttonStates" value="1"/>
        <param name="buttonStates" value="0"/>
        <param name="buttonStates" value="1"/>
        <param name="buttonStates" value="0"/>
      </TimeSampleButtonsTracker>
    </Controller>
    
  </BaseEstimator>
</config>
)XML";
    gmCore::Configuration config(xml);

    std::shared_ptr<gmTrack::BaseEstimator> tracker;
    config.getObject(tracker);

    std::vector<std::shared_ptr<gmCore::Object>> objects;
    config.getAllObjects(objects);

    for (int idx = 1; idx <= 9; ++idx) {
      gmCore::Updateable::updateAll();
    }

    Eigen::Matrix4f Raw;
    Eigen::Matrix4f Unit;

    EXPECT_TRUE(tracker->getRegistration(&Raw, &Unit));

    Eigen::Matrix4f RegA;
    RegA <<
      0, 0.5, 0, 1,
      0.5, 0, 0, 2,
      0, 0, 0.5, 1.5,
      0, 0, 0, 1;

    Eigen::Matrix4f RegB;
    RegB <<
      0,   1,   0,   0,
      1,   0,   0, 2.25,
      0,   0,   1,   0,
      0,   0,   0,   1;

    EXPECT_LT((Raw - RegA).norm(), 1e-5);
    EXPECT_LT((Unit - RegB).norm(), 1e-5);
  }
}
