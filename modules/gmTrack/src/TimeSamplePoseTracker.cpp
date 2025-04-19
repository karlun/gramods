
#include <gmTrack/TimeSamplePoseTracker.hh>

#include <gmCore/Console.hh>
#include <gmCore/Updateable.hh>
#include <gmCore/RunOnce.hh>

#include <chrono>

BEGIN_NAMESPACE_GMTRACK;

GM_OFI_DEFINE(TimeSamplePoseTracker);
GM_OFI_PARAM2(TimeSamplePoseTracker, time, double, addTime);
GM_OFI_PARAM2(TimeSamplePoseTracker, position, Eigen::Vector3f, addPosition);
GM_OFI_PARAM2(TimeSamplePoseTracker, orientation, Eigen::Quaternionf, addOrientation);


struct TimeSamplePoseTracker::Impl : gmCore::Updateable {

  Impl() : gmCore::Updateable(100) {}

  void update(clock::time_point now, size_t frame) override;

  void addTime(double t);
  void addPosition(Eigen::Vector3f p);
  void addOrientation(Eigen::Quaternionf q);

  bool getPose(PoseSample &p);

  std::vector<double> time;
  size_t sample_idx = 0;
  std::vector<Eigen::Vector3f> position;
  std::vector<Eigen::Quaternionf> orientation;

  std::optional<PoseSample> pose_sample;

  std::optional<std::chrono::steady_clock::time_point> start_time;
};

TimeSamplePoseTracker::TimeSamplePoseTracker()
  : _impl(new Impl()) {}

TimeSamplePoseTracker::~TimeSamplePoseTracker() {}

void TimeSamplePoseTracker::Impl::addTime(double t) { time.push_back(t); }

void TimeSamplePoseTracker::Impl::addPosition(Eigen::Vector3f p) { position.push_back(p); }

void TimeSamplePoseTracker::Impl::addOrientation(Eigen::Quaternionf q) { orientation.push_back(q); }

bool TimeSamplePoseTracker::Impl::getPose(PoseSample &p) {
  if (!pose_sample) return false;
  p = *pose_sample;
  return true;
}

void TimeSamplePoseTracker::Impl::update(clock::time_point now, size_t frame) {

  if (position.empty() && orientation.empty()) {
    GM_RUNONCE(GM_ERR("TimeSamplePoseTracker", "No position or orientation available."));
    return;
  }

  if (!time.empty() &&
      ((position.size() >= 2 && time.size() != position.size()) ||
       (orientation.size() >= 2 && time.size() != orientation.size()))) {

    GM_RUNONCE(GM_ERR("TimeSamplePoseTracker", "cannot calculate pose - sample count mismatch (" << time.size() << ", " << position.size() << " and " << orientation.size() << ")"));

    return;
  }

  if (!start_time) start_time = now;
  pose_sample = PoseSample {};
  pose_sample->time = now;

  if (time.empty()) {
    // If there is no time specified, iterate per frame

    size_t N = std::max(position.size(), orientation.size());

    if (position.size() == 0) {
      pose_sample->position = Eigen::Vector3f::Zero();
    } else if (position.size() == 1) {
      pose_sample->position = position[0];
    } else {
      assert(sample_idx < position.size());
      pose_sample->position = position[sample_idx];
    }

    if (orientation.size() == 0) {
      pose_sample->orientation = Eigen::Quaternionf::Identity();
    } else if (orientation.size() == 1) {
      pose_sample->orientation = orientation[0];
    } else {
      assert(sample_idx < orientation.size());
      pose_sample->orientation = orientation[sample_idx];
    }

    sample_idx = (sample_idx + 1) % N;

    return;
  }

  typedef std::chrono::duration<double, std::ratio<1>> d_seconds;
  double duration = std::chrono::duration_cast<d_seconds>
    (now - *start_time).count();

  if (duration > time.back()) {
    duration -= int(duration/time.back()) * time.back();
  }

  if (duration <= time.front()) {
    pose_sample->position = position.empty() ? Eigen::Vector3f::Zero() : position.front();
    pose_sample->orientation = orientation.empty() ? Eigen::Quaternionf::Identity()
                                        : orientation.front();
    return;
  }

  size_t to_time = 0;
  for (size_t idx = 0; idx < time.size(); ++idx) {
    if (time[idx] > duration) {
      to_time = idx;
      break;
    }
  }

  size_t from_time = to_time - 1;
  float a =
      float((duration - time[from_time]) / (time[to_time] - time[from_time]));

  if (position.empty()) {
    pose_sample->position = Eigen::Vector3f::Zero();
  } else if (position.size() == 1) {
    pose_sample->position = position.front();
  } else {
    pose_sample->position =
        ((a)*position[to_time] + (1 - a) * position[from_time]);
  }

  if (orientation.empty()) {
    pose_sample->orientation = Eigen::Quaternionf::Identity();
  } else if (orientation.size() == 1) {
    pose_sample->orientation = orientation.front();
  } else {
    pose_sample->orientation =
        orientation[from_time].slerp(a, orientation[to_time]);
  }
}

void TimeSamplePoseTracker::addTime(double t) {
  _impl->addTime(t);
}

void TimeSamplePoseTracker::addPosition(Eigen::Vector3f p) {
  _impl->addPosition(p);
}

void TimeSamplePoseTracker::addOrientation(Eigen::Quaternionf q) {
  _impl->addOrientation(q);
}

bool TimeSamplePoseTracker::getPose(PoseSample &p) {
  return _impl->getPose(p);
}

END_NAMESPACE_GMTRACK;

