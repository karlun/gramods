
#include <gmTrack/BinaryKeyChangeTracker.hh>

#include <gmCore/MathConstants.hh>

using namespace gramods;


TEST(gmTrackKeyChangeTracker, Simple) {

  auto bin_tracker = std::make_shared<gmTrack::BinaryTimeSampleTracker>();
  bin_tracker->addKeyValue("A", false);
  bin_tracker->initialize();

  auto key_change_tracker1 = std::make_shared<gmTrack::BinaryKeyChangeTracker>();
  key_change_tracker1->addTracker(bin_tracker);
  key_change_tracker1->addMapping({"A", "B"});
  key_change_tracker1->addMapping({"A", "C"});
  key_change_tracker1->initialize();

  auto key_change_tracker2 = std::make_shared<gmTrack::BinaryKeyChangeTracker>();
  key_change_tracker2->addTracker(bin_tracker);
  key_change_tracker2->addMapping({"A", "B"});
  key_change_tracker2->addMapping({"A", "C"});
  key_change_tracker2->setRetainMapped(true);
  key_change_tracker2->initialize();

  gmCore::Updateable::updateAll();

  auto state1 = key_change_tracker1->get();
  ASSERT_TRUE(state1);
  EXPECT_FALSE(state1->contains("A"));
  EXPECT_TRUE(state1->contains("B"));
  EXPECT_TRUE(state1->contains("C"));

  auto state2 = key_change_tracker2->get();
  ASSERT_TRUE(state2);
  EXPECT_TRUE(state2->contains("A"));
  EXPECT_TRUE(state2->contains("B"));
  EXPECT_TRUE(state2->contains("C"));
}
