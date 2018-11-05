
#include <gmTrack/BaseEstimator.hh>

#include <gmTrack/TimeSamplePoseTracker.hh>

#include <gmCore/Updateable.hh>

#include <gmCore/Console.hh>
#include <gmCore/OStreamMessageSink.hh>
#include <gmCore/Configuration.hh>

using namespace gramods;

TEST(gmTrackBaseEstimation, Unit) {

  gmCore::Console::removeAllSinks();
#if 1
  std::shared_ptr<gmCore::OStreamMessageSink> osms =
    std::make_shared<gmCore::OStreamMessageSink>();
  osms->initialize();
#endif

  {
    std::string xml = R"XML(
<config>

  <ImportLibrary lib="libgmTrack.so"/>

  <BaseEstimator>

    <param name="point" value="3 0 1"/>
    <param name="point" value="0 3 1"/>
    <!--param name="point" value="0 0 3"/-->
    <param name="point" value="0 0 1"/>
    <param name="point" value="3 3 1"/>

    <Controller>

      <TimeSamplePoseTracker>
        <param name="position" value="2 0 0"/>
        <param name="position" value="0 2 0"/>
        <!--param name="position" value="0 0 2"/-->
        <param name="position" value="0 0 0"/>
        <param name="position" value="2 2 0"/>
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

    std::vector<std::shared_ptr<gmCore::Object>> objects;
    config.getAllObjects(objects);

    for (int idx = 1; idx <= 9; ++idx) {
      gmCore::Updateable::updateAll();
    }
  }
}
