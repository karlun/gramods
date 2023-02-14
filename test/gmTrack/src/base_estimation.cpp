
#include <gmTrack/PoseRegistrationEstimator.hh>

#include <gmTrack/TimeSamplePoseTracker.hh>
#include <gmTrack/TimeSampleButtonsTracker.hh>

#include <gmCore/Updateable.hh>

#include <gmCore/Console.hh>
#include <gmCore/OStreamMessageSink.hh>
#include <gmCore/NullMessageSink.hh>

#include <random>

using namespace gramods;

TEST(gmTrackBaseEstimation, FullSamplesByInverse) {

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

    auto ts_buttons_tracker = std::make_shared<gmTrack::TimeSampleButtonsTracker>();
    ts_buttons_tracker->addButtons(0);
    ts_buttons_tracker->addButtons(1);
    ts_buttons_tracker->addButtons(0);
    ts_buttons_tracker->addButtons(1);
    ts_buttons_tracker->addButtons(0);
    ts_buttons_tracker->addButtons(1);
    ts_buttons_tracker->addButtons(0);
    ts_buttons_tracker->addButtons(1);
    ts_buttons_tracker->addButtons(0);
    ts_buttons_tracker->initialize();

    auto ts_pose_tracker = std::make_shared<gmTrack::TimeSamplePoseTracker>();
    ts_pose_tracker->initialize();

    auto controller = std::make_shared<gmTrack::Controller>();
    controller->setSinglePoseTracker(ts_pose_tracker);
    controller->setButtonsTracker(ts_buttons_tracker);
    controller->initialize();

    auto registrator = std::make_shared<gmTrack::PoseRegistrationEstimator>();
    registrator->setController(controller);
    registrator->initialize();

    Eigen::Matrix4f RegA;
    RegA <<
      0, 0.5, 0, 1,
      -0.5, 0, 0, 2,
      0, 0, 0.5, 3,
      0, 0, 0, 1;

    Eigen::Matrix4f RegB;
    RegB <<
      0,   1,   0, 0.5,
      -1,   0,   0, 2.5,
      0,   0,   1, 4.5,
      0,   0,   0,   1;

    Eigen::Matrix4f points;
    points <<
      3, 0, 0, 3,
      0, 3, 0, 3,
      0, 0, 3, 3,
      1, 1, 1, 1;

    Eigen::Matrix4f samples = RegA.inverse() * points;

    for (int idx = 0; idx < 4; ++idx)
      registrator->addActualPosition({points(0, idx),
                                      points(1, idx),
                                      points(2, idx)});
    for (int idx = 0; idx < 4; ++idx)
      ts_pose_tracker->addPosition({samples(0, idx),
                                    samples(1, idx),
                                    samples(2, idx)});

    for (int idx = 1; idx <= 9; ++idx) {
      gmCore::Updateable::updateAll();
    }

    Eigen::Matrix4f Raw;
    Eigen::Matrix4f Unit;

    EXPECT_TRUE(registrator->getRegistration(&Raw, &Unit));

    EXPECT_LT((Raw - RegA).norm(), 1e-5);
    EXPECT_LT((Unit - RegB).norm(), 1e-5);
  }
}

TEST(gmTrackBaseEstimation, OverDeterminedSamplesByQR) {

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

    auto ts_buttons_tracker = std::make_shared<gmTrack::TimeSampleButtonsTracker>();
    ts_buttons_tracker->addButtons(0);
    ts_buttons_tracker->addButtons(1);
    ts_buttons_tracker->addButtons(0);
    ts_buttons_tracker->addButtons(1);
    ts_buttons_tracker->addButtons(0);
    ts_buttons_tracker->addButtons(1);
    ts_buttons_tracker->addButtons(0);
    ts_buttons_tracker->addButtons(1);
    ts_buttons_tracker->addButtons(0);
    ts_buttons_tracker->initialize();

    auto ts_pose_tracker = std::make_shared<gmTrack::TimeSamplePoseTracker>();
    ts_pose_tracker->initialize();

    auto controller = std::make_shared<gmTrack::Controller>();
    controller->setSinglePoseTracker(ts_pose_tracker);
    controller->setButtonsTracker(ts_buttons_tracker);
    controller->initialize();

    auto registrator = std::make_shared<gmTrack::PoseRegistrationEstimator>();
    registrator->setController(controller);
    registrator->initialize();

    Eigen::Matrix4f RegA;
    RegA <<
      0, 0.5, 0, 1,
      -0.5, 0, 0, 2,
      0, 0, 0.5, 1.5,
      0, 0, 0, 1;

    Eigen::Matrix4f RegB;
    RegB <<
      0,   1,   0,   0,
      -1,   0,   0, 2.25,
      0,   0,   1,   0,
      0,   0,   0,   1;

    Eigen::Matrix4f points;
    points <<
      3, 0, 2, 3,
      0, 3, 1, 3,
      3, 3, 3, 3,
      1, 1, 1, 1;

    Eigen::Matrix4f samples = RegA.inverse() * points;

    for (int idx = 0; idx < 4; ++idx)
      registrator->addActualPosition({points(0, idx),
                                      points(1, idx),
                                      points(2, idx)});
    for (int idx = 0; idx < 4; ++idx)
      ts_pose_tracker->addPosition({samples(0, idx),
                                    samples(1, idx),
                                    samples(2, idx)});

    for (int idx = 1; idx <= 9; ++idx) {
      gmCore::Updateable::updateAll();
    }

    Eigen::Matrix4f Raw;
    Eigen::Matrix4f Unit;

    EXPECT_TRUE(registrator->getRegistration(&Raw, &Unit));

    EXPECT_LT((Raw - RegA).norm(), 1e-5);
    EXPECT_LT((Unit - RegB).norm(), 1e-5);
  }
}

#define LOOP_COUNT 100
#define MAX_POSITION_COUNT 20

TEST(gmTrackBaseEstimation, RandomTesting) {

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

  std::default_random_engine random_engine;
  std::uniform_real_distribution<float> real_random =
      std::uniform_real_distribution<float>(-1.f, 1.f);
  std::uniform_int_distribution<size_t> position_count_random =
      std::uniform_int_distribution<size_t>(3, MAX_POSITION_COUNT);

  for (size_t idx = 0; idx < LOOP_COUNT; ++idx) {

    Eigen::Matrix4f M_actual = Eigen::Matrix4f::Identity();
    do {

      Eigen::Vector3f rot_axis(real_random(random_engine),
                               real_random(random_engine),
                               real_random(random_engine));
      if (rot_axis.norm() < 0.1) continue;
      rot_axis = rot_axis.normalized();

      float scale = real_random(random_engine);
      if (scale < 0.1) continue;

      Eigen::Affine3f M = Eigen::Affine3f::Identity();
      M.translate(10.f * Eigen::Vector3f(real_random(random_engine),
                                         real_random(random_engine),
                                         real_random(random_engine)))
          .scale(Eigen::Vector3f(scale, scale, scale))
          .rotate(Eigen::AngleAxis<float>(3.1416 * real_random(random_engine),
                                          rot_axis))
          .translate(10.f * Eigen::Vector3f(real_random(random_engine),
                                            real_random(random_engine),
                                            real_random(random_engine)));
      M_actual = M.matrix();
    } while (M_actual.norm() < 0.1f);

    size_t position_count = position_count_random(random_engine);

    std::vector<Eigen::Vector3f> tracker_positions;
    tracker_positions.reserve(position_count);
    for (size_t idx = 0; idx < position_count; ++idx)
      tracker_positions.push_back({real_random(random_engine),
                                   real_random(random_engine),
                                   real_random(random_engine)});

    gmTrack::PoseRegistrationEstimator registrator;
    for (auto &pt : tracker_positions) {
      registrator.addTrackerPosition(pt);
      registrator.addActualPosition(
          (M_actual * pt.homogeneous()).hnormalized());
    }
    registrator.performRegistration();

    Eigen::Matrix4f M_raw;

    bool has_reg = registrator.getRegistration(&M_raw, nullptr);
    EXPECT_TRUE(has_reg);
    if (has_reg) EXPECT_LT((M_actual - M_raw).norm(), 1e-5);
  }
}
