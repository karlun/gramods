
#include <gmCore/Console.hh>

#include <gmTouch/TouchState.hh>
#include <gmCore/OStreamMessageSink.hh>
#include <gmCore/NullMessageSink.hh>

#include <thread>

using namespace gramods;

namespace {

struct MyEventAdaptor : gmTouch::TouchState::EventAdaptor {

  void touch(size_t id, float x, float y) {
    addTouchState(id, x, y);
  }

  void untouch(size_t id, float x, float y) {
    removeTouchState(id, x, y);
  }
};
}

TEST(gmTouchFingers, SimpleFingers) {

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

  gmTouch::TouchState state;

  state.eventsInit(1000, 1000);
  state.eventsDone();

  {
    gmTouch::TouchState::TouchPoints current;
    EXPECT_EQ(0, state.getTouchPoints(current));
  }

  state.eventsInit(1000, 1000);
  { MyEventAdaptor & adaptor = state.getEventAdaptor<MyEventAdaptor>();
    adaptor.touch(5, 0, 0); }
  state.eventsDone();

  {
    gmTouch::TouchState::TouchPoints current;
    EXPECT_EQ(1, state.getTouchPoints(current));
    gmTouch::TouchState::TouchPoints last;
    EXPECT_EQ(0, state.getTouchPoints(last, current));
  }

  state.eventsInit(1000, 1000);
  { MyEventAdaptor & adaptor = state.getEventAdaptor<MyEventAdaptor>();
    adaptor.touch(5, 1, 0); }
  state.eventsDone();

  {
    gmTouch::TouchState::TouchPoints current;
    EXPECT_EQ(1, state.getTouchPoints(current));
    gmTouch::TouchState::TouchPoints last;
    EXPECT_EQ(1, state.getTouchPoints(last, current));
  }
}


TEST(gmTouchFingers, Association) {

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

  gmTouch::TouchState state;

  /// FRAME ---

  state.eventsInit(1000, 1000);
  { MyEventAdaptor & adaptor = state.getEventAdaptor<MyEventAdaptor>();
    adaptor.touch(7, 3, 0); }
  state.eventsDone();

  { std::map< void *, gmTouch::TouchState::TouchPoints> current;
    EXPECT_TRUE(state.getTouchPoints(current));
    EXPECT_EQ(1, current[nullptr].size());
    EXPECT_EQ(0, current[(void*)5].size()); }

  state.setAssociation(7, (void*)5);

  { std::map< void *, gmTouch::TouchState::TouchPoints> current;
    EXPECT_TRUE(state.getTouchPoints(current));
    EXPECT_EQ(0, current[nullptr].size());
    EXPECT_EQ(1, current[(void*)5].size()); }

  /// FRAME ---

  state.eventsInit(1000, 1000);
  { MyEventAdaptor & adaptor = state.getEventAdaptor<MyEventAdaptor>();
    adaptor.touch(7, 3, 0); }
  state.eventsDone();

  state.setAssociation(7, (void*)7);

  { std::map< void *, gmTouch::TouchState::TouchPoints> current;
    EXPECT_TRUE(state.getTouchPoints(current));
    EXPECT_EQ(0, current[nullptr].size());
    EXPECT_EQ(1, current[(void*)7].size());
    EXPECT_EQ(1, current[(void*)5].size());
    ASSERT_EQ(1, current[(void*)7].size());
    ASSERT_EQ(1, current[(void*)5].size());
    EXPECT_NE(0, current[(void*)5][0].state & gmTouch::TouchState::State::RELEASE);
    EXPECT_EQ(0, current[(void*)7][0].state & gmTouch::TouchState::State::RELEASE); }

}
