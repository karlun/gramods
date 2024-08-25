
#include <gmTrack/RelativeTrackerPoseRegistrationEstimator.hh>

#include <gmTrack/TimeSamplePoseTracker.hh>
#include <gmTrack/TimeSampleButtonsTracker.hh>

#include <gmCore/Updateable.hh>

#include <gmCore/Console.hh>
#include <gmCore/OStreamMessageSink.hh>
#include <gmCore/NullMessageSink.hh>
#include <gmCore/MathConstants.hh>

#include <random>

using namespace gramods;
using namespace std::literals::chrono_literals;

TEST(gmTrackPosesEstimation, Basic) {

  gmCore::Console::removeAllSinks();
#if 0
  std::shared_ptr<gmCore::OStreamMessageSink> osms =
      std::make_shared<gmCore::OStreamMessageSink>();
  osms->setLevel(5);
  osms->initialize();
#else
  std::shared_ptr<gmCore::NullMessageSink> nullsink =
      std::make_shared<gmCore::NullMessageSink>();
  nullsink->initialize();
#endif

#define QAA(A, V) Eigen::Quaternionf(Eigen::AngleAxisf(A, V))
  std::vector<Eigen::Vector3f> Pt {{1.f, 1.f, 1.f}, //
                                   {2.f, 1.f, 4.f}, //
                                   {1.f, -1.f, 2.f}, //
                                   {3.f, 2.f, 1.f}, //
                                   {-4.f, 1.f, 1.f}};
  const double pih = GM_PI_2;
  std::vector<Eigen::Quaternionf> Qt {QAA(1 * pih, Eigen::Vector3f::UnitY()),
                                      QAA(1 * pih, Eigen::Vector3f::UnitY()),
                                      QAA(2 * pih, Eigen::Vector3f::UnitX()),
                                      QAA(1 * pih, Eigen::Vector3f::UnitZ()),
                                      QAA(3 * pih, Eigen::Vector3f::UnitY())};

  std::default_random_engine random_engine;
  std::uniform_real_distribution<float> real_random =
      std::uniform_real_distribution<float>(-1.f, 1.f);

#define RandVec(R)                                                             \
  (Eigen::Vector3f(real_random(random_engine),                                 \
                   real_random(random_engine),                                 \
                   real_random(random_engine)) *                               \
   (R))
#define RandQuat(R)                                                            \
  Eigen::Quaternionf::Identity().slerp(R, Eigen::Quaternionf::UnitRandom())

  std::vector<std::shared_ptr<gmTrack::TimeSamplePoseTracker>> trackers;
  for (const auto &p : Pt)
    trackers.push_back(std::make_shared<gmTrack::TimeSamplePoseTracker>());

#define SAMPLE_COUNT 100
#define POS_RANDOM   0.001f
#define ROT_RANDOM   0.001f

  for (size_t idx = 0; idx < SAMPLE_COUNT; ++idx) {
    for (size_t t_idx = 0; t_idx < trackers.size(); ++t_idx) {
      trackers[t_idx]->addPosition(Pt[t_idx] + RandVec(POS_RANDOM));
      trackers[t_idx]->addOrientation(Qt[t_idx] * RandQuat(ROT_RANDOM));
    }
  }

  for (auto t : trackers) t->initialize();

  auto registrator =
      std::make_shared<gmTrack::RelativeTrackerPoseRegistrationEstimator>();
  for (auto t : trackers) registrator->addSinglePoseTracker(t);
  registrator->initialize();

  gmCore::Updateable::clock::time_point sample_time =
      gmCore::Updateable::clock::now();
  for (int idx = 0; idx < SAMPLE_COUNT; ++idx) {
    sample_time += 1s;
    gmCore::Updateable::updateAll(sample_time);
  }

  typedef gmTrack::RelativeTrackerPoseRegistrationEstimator::Pose Pose;

  std::vector<Pose> Xr;
  for (size_t idx = 0; idx < registrator->getSinglePoseTrackerCount(); ++idx)
    Xr.push_back(registrator->getSinglePoseTrackerPose(idx));

  ASSERT_EQ(Xr.size(), trackers.size()) << "Size of result";

  for (auto &X : Xr) {
    X.orientation = Qt[0] * X.orientation;
    X.position = Pt[0] + Qt[0] * X.position;
  }

  for (size_t idx1 = 0; idx1 < trackers.size(); ++idx1) {
    for (size_t idx2 = idx1 + 1; idx2 < trackers.size(); ++idx2) {
      Eigen::Quaternionf dQt = Qt[idx2] * Qt[idx1].conjugate();
      Eigen::Vector3f dPt = Qt[idx1].conjugate() * (Pt[idx2] - Pt[idx1]);

      Eigen::Quaternionf dQr =
          Xr[idx2].orientation * Xr[idx1].orientation.conjugate();
      Eigen::Vector3f dPr = Xr[idx1].orientation.conjugate() *
                            (Xr[idx2].position - Xr[idx1].position);

      EXPECT_LT((dPt - dPr).norm(), 1e-2)
          << "in position " << idx1 << "-" << idx2;
      EXPECT_LT(Eigen::AngleAxisf(dQt * dQr.conjugate()).angle(), 1e-2)
          << "in orientation " << idx1 << "-" << idx2;
    }
  }
}
