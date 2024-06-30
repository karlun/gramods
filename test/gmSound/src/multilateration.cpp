
#include <gmSound/Multilateration.hh>
#include <gmSound/SoundBurstDetector.hh>

#include <gmCore/Console.hh>
#include <gmCore/OStreamMessageSink.hh>
#include <gmCore/TimeTools.hh>

using namespace gramods;

#define TOTAL_SECONDS 10
#define SAMPRATE 1000
#define SPEED 12
#define T0_SECONDS 5.1
#define SAMPLES_PER_FRAME 5

struct TestCapture : gmSound::Capture {

  TestCapture(std::vector<float> distances) {
    data.assign(4 * SAMPRATE * TOTAL_SECONDS, 0.f);

    for (size_t ch = 0; ch < 4; ++ch) {
      size_t dix = size_t(SAMPRATE * (T0_SECONDS + distances[ch] / SPEED));
      if (dix >= SAMPRATE * TOTAL_SECONDS) throw "TEST ERROR";
      data[4 * dix + ch] += 0.9f;
    }
  }

  size_t getSampleRate() override { return SAMPRATE; }

  size_t getChannelCount() override { return 4; }

  bool isOpen() override { return true; }

  void startCapture() override { is_started = true; }
  void stopCapture() override { is_started = false; }

  size_t getAvailableSamplesCount() override {
    return std::min(size_t(SAMPLES_PER_FRAME), data.size() - pos);
  }

  std::vector<float> getAvailableSamples() override {
    if (pos >= data.size()) return {};

    std::vector<float> res;

    size_t len = getAvailableSamplesCount();
    res.insert(res.end(), data.begin() + pos, data.begin() + pos + 4 * len);
    pos += 4 * len;

    return res;
  }

  bool is_started = false;
  std::vector<float> data;
  size_t pos = 0;
  
};

TEST(gmSoundMultilateration, Positioning) {

  gmCore::Console::removeAllSinks();
#if 0
  std::shared_ptr<gmCore::OStreamMessageSink> osms =
    std::make_shared<gmCore::OStreamMessageSink>();
  osms->setUseAnsiColor(true);
  osms->setLevel(4);
  osms->initialize();
#elif 0
  std::shared_ptr<gmCore::NullMessageSink> nullsink =
    std::make_shared<gmCore::NullMessageSink>();
  nullsink->initialize();
#endif

  Eigen::Vector3f p1(-10, -10, -10);
  Eigen::Vector3f p2(+10, +10, -10);
  Eigen::Vector3f p3(-10, +10, +10);
  Eigen::Vector3f p4(+10, -10, +10);

  Eigen::Vector3f p = {3.f, -2.f, 6.f};

  std::vector<float> distances = {
      (p - p1).norm(), (p - p2).norm(), (p - p3).norm(), (p - p4).norm()};

  std::stringstream dbg_str;
  for (const auto d : distances) dbg_str << d << " ";
  GM_DBG2("gmSoundMultilateration.Positioning",
          "Test distances: " << dbg_str.str());

  auto capture = std::make_shared<TestCapture>(distances);
  EXPECT_FALSE(capture->is_started);
  capture->initialize();

  auto detector = std::make_shared<gmSound::SoundBurstDetector>();
  detector->setCapture(capture);
  detector->setWindow(5);
  detector->initialize();

  gmSound::Multilateration proc;
  proc.setSoundDetector(detector);
  proc.setSpeedOfSound(SPEED);
  proc.addPoint(p1);
  proc.addPoint(p2);
  proc.addPoint(p3);
  proc.addPoint(p4);
  proc.initialize();

  size_t samples_left = SAMPRATE * TOTAL_SECONDS;

  for (size_t idx = 0; idx < 5; ++idx) { gmCore::Updateable::updateAll(); }
  samples_left -= 5 * SAMPLES_PER_FRAME;

  EXPECT_TRUE(capture->is_started);
  gmTrack::PoseTracker::PoseSample pose;
  EXPECT_FALSE(proc.getPose(pose));

  auto time = gmCore::Updateable::clock::now();
  for (size_t idx = 0; idx < 2 * samples_left / SAMPLES_PER_FRAME; ++idx) {
    time = gmCore::Updateable::clock::now();
    gmCore::Updateable::updateAll(time);
  }

  double pose_time_ago = (TOTAL_SECONDS - T0_SECONDS);

  EXPECT_TRUE(proc.getPose(pose));
  EXPECT_LE((pose.position - p).norm(),1e-2);
  EXPECT_LE(std::fabs(gmCore::TimeTools::durationToSeconds(time - pose.time) -
                      pose_time_ago),
            1e-2)
      << gmCore::TimeTools::durationToSeconds(time - pose.time) << " - "
      << pose_time_ago;
}
