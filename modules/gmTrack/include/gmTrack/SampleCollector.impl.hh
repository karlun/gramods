
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
  bool collecting = false;

  virtual void update(clock::time_point t);

  void getAverage(std::vector<Eigen::Vector3f> samples, Eigen::Vector3f &x);
  void getAverage(std::vector<Eigen::Quaternionf> samples, Eigen::Quaternionf &x);

  std::shared_ptr<gramods::gmTrack::Controller> controller;
};

END_NAMESPACE_GMTRACK;