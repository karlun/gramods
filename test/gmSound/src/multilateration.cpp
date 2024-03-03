
#include <gmSound/Multilateration.hh>

#include <gmCore/Console.hh>
#include <gmCore/OStreamMessageSink.hh>

using namespace gramods;

#define SAMPRATE 1000
#define SPEED 12

struct TestCapture : gmSound::Capture {

  TestCapture(std::vector<float> distances) {
    data.assign(4 * SAMPRATE * 10, 0);

    for (size_t ch = 0; ch < 4; ++ch) {
      size_t dix = SAMPRATE * distances[ch] / SPEED;
      if (dix >= SAMPRATE * 10) throw "TEST ERROR";
      data[4 * dix + ch] += std::int16_t(30000);
    }
  }

  size_t getSampleRate() { return SAMPRATE; }

  size_t getChannelCount() { return 4; }

  bool isOpen() { return true; }

  void startCapture() { is_started = true; }
  void stopCapture() { is_started = false; }

  size_t getAvailableSamplesCount() {
    return std::min(size_t(5), data.size() - pos);
  }

  std::vector<std::int16_t> getAvailableSamples() {
    if (pos >= data.size()) return {};

    std::vector<std::int16_t> res;

    size_t len = getAvailableSamplesCount();
    res.insert(res.end(), data.begin() + pos, data.begin() + pos + 4 * len);
    pos += 4 * len;

    return res;
  }

  bool is_started = false;
  std::vector<std::int16_t> data;
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

  gmSound::Multilateration proc;
  proc.setCapture(capture);
  proc.setSpeedOfSound(SPEED);
  proc.setWindow(1);
  proc.addPoint(p1);
  proc.addPoint(p2);
  proc.addPoint(p3);
  proc.addPoint(p4);
  proc.initialize();

  for (int idx = 0; idx < 5; ++idx) { gmCore::Updateable::updateAll(); }

  EXPECT_TRUE(capture->is_started);
  gmTrack::PoseTracker::PoseSample pose;
  EXPECT_FALSE(proc.getPose(pose));

  for (int idx = 0; idx < 364; ++idx) { gmCore::Updateable::updateAll(); }

  EXPECT_TRUE(proc.getPose(pose));
  EXPECT_LE((pose.position - p).norm(),1e-2);

}
