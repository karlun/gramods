
#include <gmTrack/VrpnPoseTracker.hh>

#ifdef gramods_ENABLE_VRPN

#include <gmTrack/MultiToSinglePoseTracker.hh>

#include <gmCore/Console.hh>
#include <gmCore/OStreamMessageSink.hh>
#include <gmCore/NullMessageSink.hh>

#include <memory>
#include <string>
#include <sstream>

#include <chrono>
#include <thread>

#include <vrpn_Tracker.h>
#include <vrpn_ConnectionPtr.h>

using namespace gramods;

TEST(gmTrackVrpn, VrpnTracker) {

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
    vrpn_ConnectionPtr conn = vrpn_ConnectionPtr::create_server_connection(3883);
    vrpn_Tracker_Server server("TEST_DEVICE", conn.get());

    struct timeval timestamp;
    vrpn_gettimeofday(&timestamp, NULL);

    vrpn_float64 position[3];
    vrpn_float64 quaternion[4];

    position[0] = 0.1;
    position[1] = 0.2;
    position[2] = 0.3;
    
    quaternion[0] = 0.4;
    quaternion[1] = 0.5;
    quaternion[2] = 0.6;
    quaternion[3] = 0.7;
    
    server.report_pose(0, timestamp, position, quaternion);
    server.mainloop();

    std::map<int, gmTrack::PoseTracker::PoseSample> samples;
    {
      gmTrack::VrpnPoseTracker tracker;
      tracker.setConnectionString("TEST_DEVICE@localhost");
      tracker.initialize();

      for (int idx = 0; idx < 10; ++idx) {
        server.report_pose(0, timestamp, position, quaternion);
        server.mainloop();
        conn->mainloop();

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        gmCore::Updateable::updateAll();

        tracker.getPose(samples);
        GM_DBG1("Test", samples[0].position);
      }
    }

    EXPECT_EQ(1, samples.size());
    EXPECT_TRUE(samples.count(0) == 1);
    if (samples.find(0) != samples.end()) {
      Eigen::Vector3f err = samples[0].position - Eigen::Vector3f(0.1f, 0.2f, 0.3f);
      EXPECT_FLOAT_EQ(err.norm(), 0);
    }
  }
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

TEST(gmTrackVrpn, VrpnPoseTrackerConfigurationAndMultiToSinglePoseTracker) {

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
    vrpn_ConnectionPtr conn = vrpn_ConnectionPtr::create_server_connection(3883);
    vrpn_Tracker_Server server("TEST_DEVICE", conn.get());

    struct timeval timestamp;
    vrpn_gettimeofday(&timestamp, NULL);

    vrpn_float64 position[3];
    vrpn_float64 quaternion[4];

    position[0] = 0.1;
    position[1] = 0.2;
    position[2] = 0.3;
    
    quaternion[0] = 0.4;
    quaternion[1] = 0.5;
    quaternion[2] = 0.6;
    quaternion[3] = 0.7;
    
    server.report_pose(0, timestamp, position, quaternion);
    server.mainloop();

    gmTrack::PoseTracker::PoseSample sample;
    bool got_sample = false;
    {

      auto vrpn_tracker = std::make_shared<gmTrack::VrpnPoseTracker>();
      vrpn_tracker->setConnectionString("TEST_DEVICE@localhost");
      vrpn_tracker->initialize();

      auto m2s_tracker = std::make_shared<gmTrack::MultiToSinglePoseTracker>();
      m2s_tracker->setMultiPoseTracker(vrpn_tracker);
      m2s_tracker->setSensor(0);
      m2s_tracker->initialize();

      std::shared_ptr<gmTrack::SinglePoseTracker> tracker = m2s_tracker;

      for (int idx = 0; idx < 10; ++idx) {
        server.report_pose(0, timestamp, position, quaternion);
        server.mainloop();
        conn->mainloop();

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        gmCore::Updateable::updateAll();

        got_sample = tracker->getPose(sample);
      }
    }

    EXPECT_TRUE(got_sample);
  }
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

#endif
