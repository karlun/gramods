
#include <gmCore/MathConstants.hh>

BEGIN_NAMESPACE_GMTRACK;

struct SampleCollector::Impl {

  typedef gmCore::Updateable::clock clock;

  std::vector<Eigen::Vector3f> tracker_positions;
  std::vector<Eigen::Quaternionf> tracker_orientations;

  std::vector<Eigen::Vector3f> sample_positions;
  std::vector<Eigen::Quaternionf> sample_orientations;

  clock::time_point last_sample_time = clock::time_point::min();
  float samples_per_second = 1;
  float warning_threshold = 0.01f;
  float orientation_warning_threshold = GM_PI_4;
  bool collecting = false;

  float inlier_threshold = -1.f;
  float orientation_inlier_threshold = -1.f;

  virtual void update(clock::time_point time);

  std::shared_ptr<gramods::gmTrack::Controller> controller;
};

END_NAMESPACE_GMTRACK;
