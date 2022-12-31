
#include <gmTrack/PoseRegistrationEstimator.hh>

#include <gmTrack/TimeSampleButtonsTracker.hh>
#include <gmTrack/TimeSampleAnalogsTracker.hh>
#include <gmTrack/ButtonsMapper.hh>
#include <gmTrack/AnalogsMapper.hh>

#include <gmCore/Updateable.hh>

#include <gmCore/Console.hh>
#include <gmCore/OStreamMessageSink.hh>

using namespace gramods;

TEST(gmTrackMapper, Buttons) {

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

  {
    std::string xml = R"lang=xml(
<config>

  <ImportLibrary library="libgmTrack.so"/>

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

    auto ts_tracker = std::make_shared<gmTrack::TimeSampleButtonsTracker>();
    ts_tracker->addButtons(0);
    ts_tracker->addButtons(1);
    ts_tracker->addButtons(2);
    ts_tracker->addButtons(3);
    ts_tracker->addButtons(4);
    ts_tracker->initialize();

    auto tracker = std::make_shared<gmTrack::ButtonsMapper>();
    tracker->setButtonsTracker(ts_tracker);
    tracker->addMapping({2, 1});
    tracker->addMapping({1, 0});
    tracker->addMapping({0, 2});
    tracker->initialize();

    gmTrack::ButtonsTracker::ButtonsSample sample;

    // 0
    EXPECT_TRUE(tracker->getButtons(sample));
    EXPECT_EQ(0, sample.buttons.size());

    // 1 (0 -> 2)
    EXPECT_TRUE(tracker->getButtons(sample));
    EXPECT_EQ(1, sample.buttons.size());
    EXPECT_EQ(1, sample.buttons.count(2));
    EXPECT_TRUE(sample.buttons[2]);

    // 2 (1 -> 0)
    EXPECT_TRUE(tracker->getButtons(sample));
    EXPECT_EQ(1, sample.buttons.size());
    EXPECT_EQ(1, sample.buttons.count(0));
    EXPECT_TRUE(sample.buttons[0]);

    // 3 (0 -> 2 / 1 -> 0)
    EXPECT_TRUE(tracker->getButtons(sample));
    EXPECT_EQ(2, sample.buttons.size());
    EXPECT_EQ(1, sample.buttons.count(0));
    EXPECT_TRUE(sample.buttons[0]);
    EXPECT_EQ(1, sample.buttons.count(2));
    EXPECT_TRUE(sample.buttons[2]);

    // 4 (2 -> 1)
    EXPECT_TRUE(tracker->getButtons(sample));
    EXPECT_EQ(1, sample.buttons.size());
    EXPECT_EQ(1, sample.buttons.count(1));
    EXPECT_TRUE(sample.buttons[1]);
  }
}

TEST(gmTrackMapper, Buttons2) {

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

  {
    std::string xml = R"lang=xml(
<config>

  <ImportLibrary library="libgmTrack.so"/>

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

    auto ts_tracker = std::make_shared<gmTrack::TimeSampleButtonsTracker>();
    ts_tracker->addButtons(0);
    ts_tracker->addButtons(1);
    ts_tracker->addButtons(2);
    ts_tracker->addButtons(3);
    ts_tracker->addButtons(4);
    ts_tracker->initialize();

    auto tracker = std::make_shared<gmTrack::ButtonsMapper>();
    tracker->setButtonsTracker(ts_tracker);
    tracker->setMainButton(1);
    tracker->setSecondaryButton(2);
    tracker->setMenuButton(0);
    tracker->initialize();

    gmTrack::ButtonsTracker::ButtonsSample sample;

    // 0
    EXPECT_TRUE(tracker->getButtons(sample));
    EXPECT_EQ(0, sample.buttons.size());

    // 1 (0 -> 2)
    EXPECT_TRUE(tracker->getButtons(sample));
    EXPECT_EQ(1, sample.buttons.size());
    EXPECT_EQ(1, sample.buttons.count(2));
    EXPECT_TRUE(sample.buttons[2]);

    // 2 (1 -> 0)
    EXPECT_TRUE(tracker->getButtons(sample));
    EXPECT_EQ(1, sample.buttons.size());
    EXPECT_EQ(1, sample.buttons.count(0));
    EXPECT_TRUE(sample.buttons[0]);

    // 3 (0 -> 2 / 1 -> 0)
    EXPECT_TRUE(tracker->getButtons(sample));
    EXPECT_EQ(2, sample.buttons.size());
    EXPECT_EQ(1, sample.buttons.count(0));
    EXPECT_TRUE(sample.buttons[0]);
    EXPECT_EQ(1, sample.buttons.count(2));
    EXPECT_TRUE(sample.buttons[2]);

    // 4 (2 -> 1)
    EXPECT_TRUE(tracker->getButtons(sample));
    EXPECT_EQ(1, sample.buttons.size());
    EXPECT_EQ(1, sample.buttons.count(1));
    EXPECT_TRUE(sample.buttons[1]);
  }
}

TEST(gmTrackMapper, Analogs) {

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

  {
    std::string xml = R"lang=xml(
<config>

  <ImportLibrary library="libgmTrack.so"/>

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

    auto ts_tracker = std::make_shared<gmTrack::TimeSampleAnalogsTracker>();
    ts_tracker->addAnalogs({0, 0, 0});
    ts_tracker->addAnalogs({1, 2, 3});
    ts_tracker->initialize();

    auto tracker = std::make_shared<gmTrack::AnalogsMapper>();
    tracker->setAnalogsTracker(ts_tracker);
    tracker->addMapping({2, 1});
    tracker->addMapping({1, 0});
    tracker->addMapping({0, 2});
    tracker->initialize();

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
#else
  std::shared_ptr<gmCore::NullMessageSink> nullsink =
    std::make_shared<gmCore::NullMessageSink>();
  nullsink->initialize();
#endif

  {
    std::string xml = R"lang=xml(
<config>

  <ImportLibrary library="libgmTrack.so"/>

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

    auto ts_tracker = std::make_shared<gmTrack::TimeSampleAnalogsTracker>();
    ts_tracker->addAnalogs({0, 0, 0});
    ts_tracker->addAnalogs({1, 2, 3});
    ts_tracker->initialize();

    auto tracker = std::make_shared<gmTrack::AnalogsMapper>();
    tracker->setAnalogsTracker(ts_tracker);
    tracker->setVerticalAnalog(1);
    tracker->setHorizontalAnalog(2);
    tracker->setTriggerAnalog(0);
    tracker->initialize();

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
