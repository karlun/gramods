
#include <gmTrack/TimeSamplePoseTracker.hh>

#include <gmCore/Console.hh>
#include <gmCore/RunOnce.hh>

#include <chrono>

BEGIN_NAMESPACE_GMTRACK;

GM_OFI_DEFINE(TimeSamplePoseTracker);
GM_OFI_PARAM(TimeSamplePoseTracker, time, double, TimeSamplePoseTracker::addTime);
GM_OFI_PARAM(TimeSamplePoseTracker, position, Eigen::Vector3f, TimeSamplePoseTracker::addPosition);
GM_OFI_PARAM(TimeSamplePoseTracker, orientation, Eigen::Quaternionf, TimeSamplePoseTracker::addOrientation);


struct TimeSamplePoseTracker::Impl {

  Impl();

  void addTime(double t);
  void addPosition(Eigen::Vector3f p);
  void addOrientation(Eigen::Quaternionf q);

  bool getPose(PoseSample &p);

  std::vector<double> time;
  size_t sample_idx;
  std::vector<Eigen::Vector3f> position;
  std::vector<Eigen::Quaternionf> orientation;

  std::chrono::steady_clock::time_point start_time;
};

TimeSamplePoseTracker::TimeSamplePoseTracker()
  : _impl(new Impl()) {}

TimeSamplePoseTracker::~TimeSamplePoseTracker() {}

TimeSamplePoseTracker::Impl::Impl()
  : start_time(std::chrono::steady_clock::now()),
    sample_idx(-1) {}


void TimeSamplePoseTracker::Impl::addTime(double t) { time.push_back(t); }

void TimeSamplePoseTracker::Impl::addPosition(Eigen::Vector3f p) { position.push_back(p); }

void TimeSamplePoseTracker::Impl::addOrientation(Eigen::Quaternionf q) { orientation.push_back(q); }

bool TimeSamplePoseTracker::Impl::getPose(PoseSample &p) {

  if (!time.empty() &&
      ((position.size() >= 2 && time.size() != position.size()) ||
       (orientation.size() >= 2 && time.size() != orientation.size()))) {

    GM_RUNONCE(GM_ERR("TimeSamplePoseTracker", "cannot calculate pose - sample count mismatch (" << time.size() << ", " << position.size() << " and " << orientation.size() << ")"));

    return false;
  }

  auto now = std::chrono::steady_clock::now();
  p.time = now;

  if (time.empty()) {
    // If there is not time specified, iterate per frame

    size_t N = std::max(position.size(), orientation.size());
    sample_idx = (sample_idx + 1) % N;

    if (position.size() == 0) {
      p.position = Eigen::Vector3f::Zero();
    } else if (position.size() == 1) {
      p.position = position[0];
    } else {
      assert(sample_idx < position.size());
      p.position = position[sample_idx];
    }

    if (orientation.size() == 0) {
      p.orientation = Eigen::Quaternionf::Identity();
    } else if (orientation.size() == 1) {
      p.orientation = orientation[0];
    } else {
      assert(sample_idx < orientation.size());
      p.orientation = orientation[sample_idx];
    }

    return true;
  }

  typedef std::chrono::duration<double, std::ratio<1>> d_seconds;
  auto duration = std::chrono::duration_cast<d_seconds>
    (now - start_time).count();

  if (duration > time.back()) {
    duration -= int(duration/time.back()) * time.back();
  }

  size_t from_time = -1;
  for (int idx = 0; idx < time.size(); ++idx) {
    if (time[idx] > duration) {
      from_time = idx;
      break;
    }
  }
  assert(from_time >= 0);

  int to_time = from_time + 1;
  assert(to_time < time.size());

  double a = (duration - time[from_time]) / (time[to_time] - time[from_time]);

  if (position.size() == 0) {
    p.position = Eigen::Vector3f::Zero();
  } else if (position.size() == 1) {
    p.position = position[0];
  } else {
    p.position = (( a ) * position[to_time] +
                  (1-a) * position[from_time]);
  }

  if (orientation.size() == 0) {
    p.orientation = Eigen::Quaternionf::Identity();
  } else if (orientation.size() == 1) {
    p.orientation = orientation[0];
  } else {
    p.orientation = orientation[from_time].slerp(a, orientation[to_time]);
  }

  return true;
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

