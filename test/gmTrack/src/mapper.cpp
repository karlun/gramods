
#include <gmTrack/PoseRegistrationEstimator.hh>

#include <gmTrack/AnalogsMapper.hh>

#include <gmCore/Updateable.hh>

#include <gmCore/Console.hh>
#include <gmCore/OStreamMessageSink.hh>
#include <gmCore/Configuration.hh>

using namespace gramods;

TEST(gmTrackMapper, Buttons) {

  gmCore::Console::removeAllSinks();
#if 0
  std::shared_ptr<gmCore::OStreamMessageSink> osms =
    std::make_shared<gmCore::OStreamMessageSink>();
  osms->initialize();
#endif

  {
    std::string xml = R"lang=xml(
<config>

  <ImportLibrary lib="libgmTrack.so"/>

  <ButtonsMapper>

    <param name="mapping" value="2 1"/>
    <param name="mapping" value="1 0"/>
    <param name="mapping" value="0 2"/>

    <TimeSampleButtonsTracker>

      <param name="buttons" value="0"/>
      <param name="buttons" value="1"/>
      <param name="buttons" value="2"/>
      <param name="buttons" value="3"/>
      <param name="buttons" value="4"/>

    </TimeSampleButtonsTracker>
  </ButtonsMapper>

</config>
)lang=xml";
    gmCore::Configuration config(xml);

    std::shared_ptr<gmTrack::ButtonsTracker> tracker;
    config.getObject(tracker);

    std::vector<std::shared_ptr<gmCore::Object>> objects;
    config.getAllObjects(objects);

    gmTrack::ButtonsTracker::ButtonsSample sample;

    EXPECT_TRUE(tracker->getButtons(sample));
    EXPECT_EQ(0, sample.buttons);

    EXPECT_TRUE(tracker->getButtons(sample));
    EXPECT_EQ(4, sample.buttons);

    EXPECT_TRUE(tracker->getButtons(sample));
    EXPECT_EQ(1, sample.buttons);

    EXPECT_TRUE(tracker->getButtons(sample));
    EXPECT_EQ(5, sample.buttons);

    EXPECT_TRUE(tracker->getButtons(sample));
    EXPECT_EQ(2, sample.buttons);
  }
}

TEST(gmTrackMapper, Buttons2) {

  gmCore::Console::removeAllSinks();
#if 0
  std::shared_ptr<gmCore::OStreamMessageSink> osms =
    std::make_shared<gmCore::OStreamMessageSink>();
  osms->initialize();
#endif

  {
    std::string xml = R"lang=xml(
<config>

  <ImportLibrary lib="libgmTrack.so"/>

  <ButtonsMapper>

    <param name="mainButton" value="1"/>
    <param name="secondaryButton" value="2"/>
    <param name="menuButton" value="0"/>

    <TimeSampleButtonsTracker>

      <param name="buttons" value="0"/>
      <param name="buttons" value="1"/>
      <param name="buttons" value="2"/>
      <param name="buttons" value="3"/>
      <param name="buttons" value="4"/>

    </TimeSampleButtonsTracker>
  </ButtonsMapper>

</config>
)lang=xml";
    gmCore::Configuration config(xml);

    std::shared_ptr<gmTrack::ButtonsTracker> tracker;
    config.getObject(tracker);

    std::vector<std::shared_ptr<gmCore::Object>> objects;
    config.getAllObjects(objects);

    gmTrack::ButtonsTracker::ButtonsSample sample;

    EXPECT_TRUE(tracker->getButtons(sample));
    EXPECT_EQ(0, sample.buttons);

    EXPECT_TRUE(tracker->getButtons(sample));
    EXPECT_EQ(4, sample.buttons);

    EXPECT_TRUE(tracker->getButtons(sample));
    EXPECT_EQ(1, sample.buttons);

    EXPECT_TRUE(tracker->getButtons(sample));
    EXPECT_EQ(5, sample.buttons);

    EXPECT_TRUE(tracker->getButtons(sample));
    EXPECT_EQ(2, sample.buttons);
  }
}

TEST(gmTrackMapper, Analogs) {

  gmCore::Console::removeAllSinks();
#if 0
  std::shared_ptr<gmCore::OStreamMessageSink> osms =
    std::make_shared<gmCore::OStreamMessageSink>();
  osms->initialize();
#endif

  {
    std::string xml = R"lang=xml(
<config>

  <ImportLibrary lib="libgmTrack.so"/>

  <AnalogsMapper>

    <param name="mapping" value="2 1"/>
    <param name="mapping" value="1 0"/>
    <param name="mapping" value="0 2"/>

    <TimeSampleAnalogsTracker>

      <param name="analogs" value="0 0 0"/>
      <param name="analogs" value="1 2 3"/>

    </TimeSampleAnalogsTracker>
  </AnalogsMapper>

</config>
)lang=xml";
    gmCore::Configuration config(xml);

    std::shared_ptr<gmTrack::AnalogsTracker> tracker;
    config.getObject(tracker);

    std::vector<std::shared_ptr<gmCore::Object>> objects;
    config.getAllObjects(objects);

    gmTrack::AnalogsTracker::AnalogsSample sample;

    EXPECT_TRUE(tracker->getAnalogs(sample));
    ASSERT_GE(3, sample.analogs.size());
    EXPECT_EQ(0, sample.analogs[gmTrack::AnalogsMapper::AnalogIndex::VERTICAL]);
    EXPECT_EQ(0, sample.analogs[gmTrack::AnalogsMapper::AnalogIndex::HORIZONTAL]);
    EXPECT_EQ(0, sample.analogs[gmTrack::AnalogsMapper::AnalogIndex::TRIGGER]);

    EXPECT_TRUE(tracker->getAnalogs(sample));
    ASSERT_GE(3, sample.analogs.size());
    EXPECT_EQ(2, sample.analogs[gmTrack::AnalogsMapper::AnalogIndex::VERTICAL]);
    EXPECT_EQ(3, sample.analogs[gmTrack::AnalogsMapper::AnalogIndex::HORIZONTAL]);
    EXPECT_EQ(1, sample.analogs[gmTrack::AnalogsMapper::AnalogIndex::TRIGGER]);

  }
}

TEST(gmTrackMapper, Analogs2) {

  gmCore::Console::removeAllSinks();
#if 0
  std::shared_ptr<gmCore::OStreamMessageSink> osms =
    std::make_shared<gmCore::OStreamMessageSink>();
  osms->initialize();
#endif

  {
    std::string xml = R"lang=xml(
<config>

  <ImportLibrary lib="libgmTrack.so"/>

  <AnalogsMapper>

    <param name="verticalAnalog" value="1"/>
    <param name="horizontalAnalog" value="2"/>
    <param name="triggerAnalog" value="0"/>

    <TimeSampleAnalogsTracker>

      <param name="analogs" value="0 0 0"/>
      <param name="analogs" value="1 2 3"/>

    </TimeSampleAnalogsTracker>
  </AnalogsMapper>

</config>
)lang=xml";
    gmCore::Configuration config(xml);

    std::shared_ptr<gmTrack::AnalogsTracker> tracker;
    config.getObject(tracker);

    std::vector<std::shared_ptr<gmCore::Object>> objects;
    config.getAllObjects(objects);

    gmTrack::AnalogsTracker::AnalogsSample sample;

    EXPECT_TRUE(tracker->getAnalogs(sample));
    ASSERT_GE(3, sample.analogs.size());
    EXPECT_EQ(0, sample.analogs[gmTrack::AnalogsMapper::AnalogIndex::VERTICAL]);
    EXPECT_EQ(0, sample.analogs[gmTrack::AnalogsMapper::AnalogIndex::HORIZONTAL]);
    EXPECT_EQ(0, sample.analogs[gmTrack::AnalogsMapper::AnalogIndex::TRIGGER]);

    EXPECT_TRUE(tracker->getAnalogs(sample));
    ASSERT_GE(3, sample.analogs.size());
    EXPECT_EQ(2, sample.analogs[gmTrack::AnalogsMapper::AnalogIndex::VERTICAL]);
    EXPECT_EQ(3, sample.analogs[gmTrack::AnalogsMapper::AnalogIndex::HORIZONTAL]);
    EXPECT_EQ(1, sample.analogs[gmTrack::AnalogsMapper::AnalogIndex::TRIGGER]);

  }
}

