
#include <gmTrack/VRPNTracker.hh>

#ifdef GM_ENABLE_VRPN

#include <gmTrack/SingleTracker.hh>

#include <gmCore/Console.hh>
#include <gmCore/OStreamMessageSink.hh>
#include <gmCore/Configuration.hh>

#include <memory>
#include <string>
#include <sstream>

#include <chrono>
#include <thread>

#include <vrpn_ConnectionPtr.h>

using namespace gramods;

TEST(gmTrackVRPN, VRPNTracker) {

#if 1
  gmCore::Console::setDefaultSink(nullptr);
#else  
  std::shared_ptr<gmCore::OStreamMessageSink> osms =
    std::make_shared<gmCore::OStreamMessageSink>();
  osms->initialize();
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

    std::map<int, gmTrack::Tracker::PoseSample> samples;
    {
      gmTrack::VRPNTracker tracker;
      tracker.setConnectionString("TEST_DEVICE@localhost");
      tracker.initialize();

      for (int idx = 0; idx < 10; ++idx) {
        server.report_pose(0, timestamp, position, quaternion);
        server.mainloop();
        conn->mainloop();

        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        tracker.getPose(samples);
        GM_INF("Test", samples[0].position);
      }
    }

    EXPECT_EQ(1, samples.size());
    EXPECT_TRUE(samples.count(0) == 1);
    if (samples.find(0) != samples.end()) {
      Eigen::Vector3f err = samples[0].position - Eigen::Vector3f(0.1, 0.2, 0.3);
      EXPECT_LE(err.norm(), std::numeric_limits<float>::epsilon());
    }
  }
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

TEST(gmTrackVRPN, VRPNTrackerConfigurationAndMultiToSingleTracker) {

#if 1
  gmCore::Console::setDefaultSink(nullptr);
#else  
  std::shared_ptr<gmCore::OStreamMessageSink> osms =
    std::make_shared<gmCore::OStreamMessageSink>();
  osms->initialize();
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

    gmTrack::Tracker::PoseSample sample;
    bool got_sample = false;
    {
      std::string xml = ""
        "<config>"
        "  <MultiToSingleTracker sensor=\"0\">"
        "    <VRPNTracker AS=\"tracker\" connectionString=\"TEST_DEVICE@localhost\"/>"
        "  </MultiToSingleTracker>"
        "</config>";
      gmCore::Configuration config(xml);

      std::shared_ptr<gmTrack::SingleTracker> tracker;
      bool got_tracker = config.getObject(tracker);

      EXPECT_TRUE(got_tracker);
      EXPECT_TRUE(tracker);

      if (!tracker)
        return;

      for (int idx = 0; idx < 10; ++idx) {
        server.report_pose(0, timestamp, position, quaternion);
        server.mainloop();
        conn->mainloop();

        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        got_sample = tracker->getPose(sample);
      }
    }

    EXPECT_TRUE(got_sample);
  }
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

#endif
