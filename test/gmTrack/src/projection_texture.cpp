
#include <gmTrack/ProjectionTextureGenerator.hh>

#include <gmTrack/TimeSamplePoseTracker.hh>
#include <gmTrack/TimeSampleButtonsTracker.hh>

#include <gmCore/Updateable.hh>
#include <gmCore/FreeImage.hh>

#include <gmCore/Console.hh>
#include <gmCore/OStreamMessageSink.hh>

#include <FreeImage.h>

using namespace gramods;

TEST(gmTrackProjectionTexture, SimpleFlat) {

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

  auto ts_buttons_tracker =
      std::make_shared<gmTrack::TimeSampleButtonsTracker>();
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

  auto file = std::filesystem::temp_directory_path() /
              (std::string(tmpnam(nullptr)) + ".tif");
  auto generator = std::make_shared<gmTrack::ProjectionTextureGenerator>();
  generator->setController(controller);
  generator->setResolution({64, 64});
  generator->setFile(file);

  generator->addBufferPosition({0.f, 0.f});
  generator->addBufferPosition({1.f, 0.f});
  generator->addBufferPosition({0.f, 1.f});
  generator->addBufferPosition({1.f, 1.f});
  ts_pose_tracker->addPosition({1.f, 1.f, 5.f});
  ts_pose_tracker->addPosition({2.f, 1.f, 5.f});
  ts_pose_tracker->addPosition({1.f, 3.f, 5.f});
  ts_pose_tracker->addPosition({2.f, 3.f, 5.f});

  generator->initialize();

  for (int idx = 1; idx <= 9; ++idx) { gmCore::Updateable::updateAll(); }

  std::filesystem::remove(file);
}

TEST(gmTrackProjectionTexture, CurvatureWRegions) {

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

  auto file = std::filesystem::temp_directory_path() /
              "dbg.tif"; //(std::string(tmpnam(nullptr)) + ".tif");
  auto generator = std::make_shared<gmTrack::ProjectionTextureGenerator>(); 
  generator->setResolution({64, 64});
  generator->setFile(file);

  auto ts_pose_tracker = std::make_shared<gmTrack::TimeSamplePoseTracker>();
  auto ts_buttons_tracker = std::make_shared<gmTrack::TimeSampleButtonsTracker>();

#define RES_X 8
#define RES_Y 8

#define FUN1_X(X, Y) (1.f + 2.f * (X))
#define FUN1_Y(X, Y) (-1.f + 2.f * (Y) + 0.1f * (X))
#define FUN1_Z(X, Y) (1.f + 0.2f * (X) + -0.6f * (Y))

  generator->addRegion(1);
  for (size_t idx_y = 0; idx_y < RES_Y; ++idx_y)
    for (size_t idx_x = 0; idx_x <= RES_X / 2; ++idx_x) {
      auto x = idx_x / (float)RES_X;
      auto y = idx_y / (float)RES_Y;
      generator->addBufferPosition({x, y});
      ts_pose_tracker->addPosition({FUN1_X(x, y), FUN1_Y(x, y), FUN1_Z(x, y)});
      ts_buttons_tracker->addButtons(0);
      ts_buttons_tracker->addButtons(1);
    }

  generator->addHullPosition({0.0f, 0.0f});
  generator->addHullPosition({0.5f, 0.0f});
  generator->addHullPosition({0.5f, 1.0f});
  generator->addHullPosition({0.0f, 1.0f});

#define FUN2_X(X, Y) (1.f + 2.f * (X))
#define FUN2_Y(X, Y) (-1.f + 2.f * (Y) + 0.1f * (X))
#define FUN2_Z(X, Y)                                                           \
  (1.f + 0.2f * (X) + -0.6f * (Y) + 1.4f * ((X)-0.5f) * ((X)-0.5f))

  generator->addRegion(2);
  for (size_t idx_y = 0; idx_y < RES_Y; ++idx_y)
    for (size_t idx_x = RES_X / 2; idx_x < RES_X; ++idx_x) {
      auto x = idx_x / (float)RES_X;
      auto y = idx_y / (float)RES_Y;
      generator->addBufferPosition({x, y});
      ts_pose_tracker->addPosition({FUN2_X(x, y), FUN2_Y(x, y), FUN2_Z(x, y)});
      ts_buttons_tracker->addButtons(0);
      ts_buttons_tracker->addButtons(1);
    }

  generator->addHullPosition({0.5f, 0.0f});
  generator->addHullPosition({1.0f, 0.0f});
  generator->addHullPosition({1.0f, 1.0f});
  generator->addHullPosition({0.5f, 1.0f});

  auto controller = std::make_shared<gmTrack::Controller>();
  controller->setSinglePoseTracker(ts_pose_tracker);
  controller->setButtonsTracker(ts_buttons_tracker);
  controller->initialize();
  generator->setController(controller);

  generator->initialize();
  ts_pose_tracker->initialize();
  ts_buttons_tracker->initialize();

  for (int idx = 1; idx <= 2 * (RES_X + 1) * RES_Y + 1; ++idx) {
    gmCore::Updateable::updateAll();
  }

  // Check results -----
  
  auto offset = generator->getOffset();
  auto scale = generator->getScale();

  std::shared_ptr<gmCore::FreeImage> free_image = gmCore::FreeImage::get();

  FREE_IMAGE_FORMAT image_format = FreeImage_GetFileType(file.u8string().c_str(), 0);
  FIBITMAP *image = FreeImage_Load(image_format, file.u8string().c_str());
  ASSERT_TRUE(image);
  ASSERT_EQ(FreeImage_GetImageType(image), FIT_RGBF);

  BYTE *image_data = FreeImage_GetBits(image);
  unsigned int image_width = FreeImage_GetWidth(image);
  unsigned int image_height = FreeImage_GetHeight(image);

  ASSERT_TRUE(image_data);
  ASSERT_TRUE(image_width);
  ASSERT_TRUE(image_height);

  float max_err = 0.f;
  float * pixels = reinterpret_cast<float*>(image_data);
#define data(X,Y,D) pixels[(D) + 3 * (X + image_width * (Y))]
  for (size_t idx_y = 0; idx_y < image_height; ++idx_y)
    for (size_t idx_x = 0; idx_x < image_width / 2; ++idx_x) {
      max_err =
          std::max(max_err,
                   std::fabs((offset[0] + scale[0] * data(idx_x, idx_y, 0)) -
                             FUN1_X(idx_x / (float)image_width,
                                    idx_y / (float)image_height)));
      max_err =
          std::max(max_err,
                   std::fabs((offset[1] + scale[1] * data(idx_x, idx_y, 1)) -
                             FUN1_Y(idx_x / (float)image_width,
                                    idx_y / (float)image_height)));
      max_err =
          std::max(max_err,
                   std::fabs((offset[2] + scale[2] * data(idx_x, idx_y, 2)) -
                             FUN1_Z(idx_x / (float)image_width,
                                    idx_y / (float)image_height)));
    }
  for (size_t idx_y = 0; idx_y < image_height; ++idx_y)
    for (size_t idx_x = image_width / 2; idx_x < image_width; ++idx_x) {
      max_err =
          std::max(max_err,
                   std::fabs((offset[0] + scale[0] * data(idx_x, idx_y, 0)) -
                             FUN2_X(idx_x / (float)image_width,
                                    idx_y / (float)image_height)));
      max_err =
          std::max(max_err,
                   std::fabs((offset[1] + scale[1] * data(idx_x, idx_y, 1)) -
                             FUN2_Y(idx_x / (float)image_width,
                                    idx_y / (float)image_height)));
      max_err =
          std::max(max_err,
                   std::fabs((offset[2] + scale[2] * data(idx_x, idx_y, 2)) -
                             FUN2_Z(idx_x / (float)image_width,
                                    idx_y / (float)image_height)));
    }

  float magn =
      (std::fabs(offset[0]) + std::fabs(offset[1]) + std::fabs(offset[2])) +
      (std::fabs(scale[0]) + std::fabs(scale[1]) + std::fabs(scale[2]));
  EXPECT_LT(max_err, magn * std::numeric_limits<float>::epsilon());

  FreeImage_Unload(image);

  //std::filesystem::remove(file);
}
