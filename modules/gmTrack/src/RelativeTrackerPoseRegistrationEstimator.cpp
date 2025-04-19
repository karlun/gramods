
#include <gmTrack/RelativeTrackerPoseRegistrationEstimator.hh>

#include <gmTrack/PoseSampleCollector.hh>

#include <gmCore/Console.hh>
#include <gmCore/MathConstants.hh>
#include <gmCore/PreConditionViolation.hh>
#include <gmCore/RunOnce.hh>
#include <gmCore/TimeTools.hh>

#include <gmCore/ExitException.hh>

using namespace std::literals::chrono_literals;

#include <gmMisc/NelderMead.hh>
#include <gmMisc/AccumulativeMean.hh>

BEGIN_NAMESPACE_GMTRACK;

GM_OFI_DEFINE(RelativeTrackerPoseRegistrationEstimator);
GM_OFI_POINTER2(RelativeTrackerPoseRegistrationEstimator, singlePoseTracker, SinglePoseTracker, addSinglePoseTracker);
GM_OFI_POINTER2(RelativeTrackerPoseRegistrationEstimator, multiPoseTracker, MultiPoseTracker, addMultiPoseTracker);
GM_OFI_PARAM2(RelativeTrackerPoseRegistrationEstimator, samplesPerSecond, float, setSamplesPerSecond);
GM_OFI_PARAM2(RelativeTrackerPoseRegistrationEstimator, warningThreshold, float, setWarningThreshold);
GM_OFI_PARAM2(RelativeTrackerPoseRegistrationEstimator, inlierThreshold, float, setInlierThreshold);
GM_OFI_PARAM2(RelativeTrackerPoseRegistrationEstimator, orientationWarningThreshold, float, setOrientationWarningThreshold);
GM_OFI_PARAM2(RelativeTrackerPoseRegistrationEstimator, orientationInlierThreshold, float, setOrientationInlierThreshold);

namespace {
struct TPose {
  TPose()
    : position(Eigen::Vector3f::Zero()),
      orientation(Eigen::Quaternionf::Identity()) {}
  TPose(const PoseTracker::PoseSample &pose)
    : position(pose.position), orientation(pose.orientation) {}
  TPose(Eigen::Vector3f position, Eigen::Quaternionf orientation)
    : position(position), orientation(orientation) {}

  operator RelativeTrackerPoseRegistrationEstimator::Pose() {
    return {position, orientation};
  }

  Eigen::Vector3f position;
  Eigen::Quaternionf orientation;
};

std::ostream &operator<<(std::ostream &out, const TPose &R) {
  Eigen::AngleAxisf aa(R.orientation);
  return out << "TPose(P=" << R.position.transpose() << ", R=" << aa.angle()
             << "/" << aa.axis().transpose() << ")";
}

/**
   Calculate a new absolute pose from one absolute pose plus a
   relative pose. B = A + R_AB
*/
TPose operator+(const TPose &A, const TPose &R) {
  return TPose(A.position + A.orientation * R.position,
               A.orientation * R.orientation);
}

/**
   Calculate a relative pose from two absolute poses. R_AB = B - A
*/
TPose operator-(const TPose &B, const TPose &A) {
  return TPose(A.orientation.conjugate() * (B.position - A.position),
               A.orientation.conjugate() * B.orientation);
}

struct TPoseList {
  void add(const TPose &p) {
    poses.push_back(p);
  }

  size_t size() const { return poses.size(); }

  TPoseList getPosMod(size_t idx, size_t dim, float d) const {
    TPoseList res;
    res.poses = poses;
    res.poses[idx].position[dim] += d;
    return res;
  }

  TPoseList getOriMod(size_t idx, size_t dim) const {
    TPoseList res;
    res.poses = poses;
    switch (dim) {
    case 0:
      res.poses[idx].orientation =
          res.poses[idx].orientation *
          Eigen::AngleAxisf(0.01f * GM_PI, Eigen::Vector3f::UnitX());
      break;
    case 1:
      res.poses[idx].orientation =
          res.poses[idx].orientation *
          Eigen::AngleAxisf(0.01f * GM_PI, Eigen::Vector3f::UnitY());
      break;
    case 2:
      res.poses[idx].orientation =
          res.poses[idx].orientation *
          Eigen::AngleAxisf(0.01f * GM_PI, Eigen::Vector3f::UnitZ());
      break;
    }
    return res;
  }

  std::vector<TPose> poses;
};

// NelderMead requires these to compile even if they are not used
TPoseList operator+(const TPoseList &A, const TPoseList &R) { throw 0; }
TPoseList operator-(const TPoseList &a, const TPoseList &b) { throw 0; }
TPoseList operator*(const TPoseList &a, float b) { throw 0; }

struct idx_t {
  idx_t(size_t tracker, size_t item) : tracker(tracker), item(int(item)) {}
  idx_t(size_t tracker, int item) : tracker(tracker), item(item) {}

  static idx_t single(size_t idx) {
    return {std::numeric_limits<size_t>::max(), int(idx)};
  }

  bool is_single() const {
    return tracker == std::numeric_limits<size_t>::max();
  }

  bool operator<(const idx_t &o2) const {
    return tracker < o2.tracker || (tracker == o2.tracker && item < o2.item);
  }

  bool operator>=(const idx_t &o2) const {
    return tracker >= o2.tracker && item >= o2.item;
  }

  bool operator==(const idx_t &o2) const {
    return tracker == o2.tracker && item == o2.item;
  }

  size_t tracker;
  int item;
};

std::ostream &operator<<(std::ostream &out, const idx_t &idx) {
  if (idx.is_single())
    return out << idx.item;
  else
    return out << idx.tracker << ":" << idx.item;
}
}

struct RelativeTrackerPoseRegistrationEstimator::Impl : gmCore::Updateable {

  Impl() : gmCore::Updateable(10) {}
  virtual ~Impl() {}

  std::vector<std::shared_ptr<SinglePoseTracker>> single_pose_trackers;
  std::vector<std::shared_ptr<MultiPoseTracker>> multi_pose_trackers;

  void update(clock::time_point time, size_t frame) override;
  std::map<std::pair<idx_t, idx_t>, gmMisc::AccumulativeMean<TPose>>
      relative_poses;

  void estimateRegistration();
  std::optional<std::map<idx_t, Pose>> relative_registration;

  clock::duration sample_delay = 100ms;
  clock::time_point last_sample_time = clock::time_point::min();

  bool print_upon_exit = true;

  float warning_threshold = 0.01f;
  float orientation_warning_threshold = GM_PI_4;

  float inlier_threshold = -1.f;
  float orientation_inlier_threshold = -1.f;
};

RelativeTrackerPoseRegistrationEstimator::
    RelativeTrackerPoseRegistrationEstimator()
  : _impl(std::make_unique<Impl>()) {}
RelativeTrackerPoseRegistrationEstimator::
    ~RelativeTrackerPoseRegistrationEstimator() {
  if (!_impl->print_upon_exit) return;

  std::stringstream out;

  if (getMultiPoseTrackerCount() > 0)
    out << "MultiPoseTracker registration\n\n";

  for (size_t idx = 0; idx < getMultiPoseTrackerCount(); ++idx) {
    const auto regs = getMultiPoseTrackerPose(idx);
    if (!regs.empty())
      out << idx << "\n";
    for (auto const &poses : regs)
      out << "  " << poses.first << " " << poses.second << "\n";
  }

  if (getSinglePoseTrackerCount() > 0)
    out << "SinglePoseTracker registration\n\n";

  for (size_t idx = 0; idx < getSinglePoseTrackerCount(); ++idx) {
    const auto pose = getSinglePoseTrackerPose(idx);
    out << "  " << idx << " " << pose << "\n";
  }
}

void RelativeTrackerPoseRegistrationEstimator::Impl::update(
    clock::time_point time, size_t frame) {

  if (time < last_sample_time + sample_delay) return;
  last_sample_time = time;

  if (multi_pose_trackers.empty() && single_pose_trackers.size() < 2) {
    GM_RUNONCE(GM_ERR("RelativeTrackerPoseRegistrationEstimator",
                      "Starting sampling with too few trackers"));
    return;
  }

  std::map<idx_t, SinglePoseTracker::PoseSample> tracker_samples;
  for (size_t idx = 0; idx < single_pose_trackers.size(); ++idx) {
    SinglePoseTracker::PoseSample sample;
    if (!single_pose_trackers[idx]->getPose(sample)) continue;
    if (sample.time + sample_delay < time) continue;
    tracker_samples[idx_t::single(idx)] = sample;
  }
  for (size_t idx = 0; idx < multi_pose_trackers.size(); ++idx) {
    std::map<int, SinglePoseTracker::PoseSample> samples;
    if (!multi_pose_trackers[idx]->getPose(samples)) continue;
    for (const auto &sample : samples) {
      if (sample.second.time + sample_delay < time) continue;
      tracker_samples[idx_t(idx, sample.first)] = sample.second;
    }
  }

  if (tracker_samples.empty()) return;

  bool change = false;
  for (const auto &sample1 : tracker_samples) {
    for (const auto &sample2 : tracker_samples) {
      if (sample1.first >= sample2.first) continue;

      std::pair<idx_t, idx_t> index {sample1.first, sample2.first};
      if (relative_poses.count(index) == 0) {
        relative_poses.emplace(
            index,
            TPose {Eigen::Vector3f::Zero(), Eigen::Quaternionf::Identity()});
        relative_poses.at({sample1.first, sample2.first}).func_interp =
            [](float r, const TPose &a, const TPose &b) -> TPose {
          return {(1 - r) * a.position + r * b.position,
                  a.orientation.slerp(r, b.orientation)};
        };
      }

      auto sampleR = TPose(sample2.second) - TPose(sample1.second);
      relative_poses.at({sample1.first, sample2.first}) += sampleR;

      change = true;
    }
  }

  if (change && relative_registration) relative_registration = std::nullopt;
}

void RelativeTrackerPoseRegistrationEstimator::Impl::estimateRegistration() {

  if (relative_poses.empty())
    throw gmCore::PreConditionViolation(
        "Cannot estimate registration with no relative samples available");

  relative_registration = std::map<idx_t, Pose>();

  std::map<idx_t, TPose> absolute_poses;
  std::vector<idx_t> untested_absolute_poses;
  untested_absolute_poses.reserve(single_pose_trackers.size() +
                                  10 * multi_pose_trackers.size());

  absolute_poses[relative_poses.begin()->first.first] = TPose();
  untested_absolute_poses.push_back(relative_poses.begin()->first.first);

  while (!untested_absolute_poses.empty()) {

    // For each untested absolut pose, check if there is a relative
    // pose available that can lead to new absolute poses.
    const auto a_idx = untested_absolute_poses.back();
    untested_absolute_poses.pop_back();

    const auto &a_pose = absolute_poses[a_idx];

    for (const auto &rpose : relative_poses) {
      if (a_idx == rpose.first.first) {
        const auto &t_idx = rpose.first.second;
        absolute_poses[t_idx] = a_pose + relative_poses.at({a_idx, t_idx});
        untested_absolute_poses.push_back(t_idx);
      }
#if 0 // Backwards use of relative pose needs work
      if (a_idx == rpose.first.second) {
        const auto &t_idx = rpose.first.first;
        absolute_poses[t_idx] = a_pose - relative_poses.at({t_idx, a_idx});
        untested_absolute_poses.push_back(t_idx);
      }
#endif
    }
  }

  for (const auto &rpose : relative_poses) {
    if (absolute_poses.count(rpose.first.first) > 0 &&
        absolute_poses.count(rpose.first.second) > 0)
      continue;
    GM_ERR("RelativeTrackerPoseRegistrationEstimator",
           "The pose relations between the trackers form disjoint groups!"
           " Registration cannot be estimated!");
    return;
  }

#if 1
  // Find appropriate size for initial simplex
  float maximum_offset = std::numeric_limits<float>::min();
  for (const auto &rpose : relative_poses)
    maximum_offset =
        std::max(maximum_offset, TPose(rpose.second).position.squaredNorm());
  float norm_dist = 0.01f * std::sqrt(maximum_offset); // 1% of workspace

  std::vector<idx_t> base_indices;
  std::map<idx_t, size_t> samples_indices;
  TPoseList BASE;

  for (auto const &pose : absolute_poses) {
    samples_indices[pose.first] = BASE.size();
    base_indices.push_back(pose.first);
    BASE.add(pose.second);
  }

  std::vector<TPoseList> X0;
  X0.reserve(6 * absolute_poses.size() + 1);
  X0.push_back(BASE);
  for (size_t idx = 0; idx < absolute_poses.size(); ++idx) {
    for (size_t dim = 0; dim < 3; ++dim) //
      X0.push_back(BASE.getPosMod(idx, dim, norm_dist));
    for (size_t dim = 0; dim < 3; ++dim) //
      X0.push_back(BASE.getOriMod(idx, dim));
  }

  gmMisc::NelderMead<float, TPoseList> solver([this, &samples_indices](
                                                  const TPoseList &X) -> float {
    float err = 0.f;
    for (const auto &rpose : relative_poses) {
      const TPose pose_diff = X.poses[samples_indices[rpose.first.second]] -
                              X.poses[samples_indices[rpose.first.first]];
      const TPose relpose(rpose.second);

      const float a = Eigen::AngleAxisf(relpose.orientation *
                                        pose_diff.orientation.conjugate())
                          .angle();
      err += a * a;
      err += (relpose.position - pose_diff.position).squaredNorm();
    }
    return std::sqrt(err);
  });

  solver.func_midpoint =
      [](const std::vector<std::pair<float, TPoseList>> &F_X) -> TPoseList {
    const size_t pose_count = F_X.front().second.poses.size();
    const size_t X_count = F_X.size() - 1;

    TPoseList res;
    res.poses.reserve(pose_count);

    const float scale = 1.f / X_count;

    for (size_t pose_idx = 0; pose_idx < pose_count; ++pose_idx) {

      Eigen::Vector3f P = Eigen::Vector3f::Zero();
      Eigen::MatrixXf Qm(4, X_count);

      for (size_t X_idx = 0; X_idx < X_count; ++X_idx) {

        P += scale * F_X[X_idx].second.poses[pose_idx].position;

        Eigen::Quaternionf pose_orientation =
            F_X[X_idx].second.poses[pose_idx].orientation;

        Eigen::MatrixXf Q(4, 1);
        Q << pose_orientation.w(), pose_orientation.x(), pose_orientation.y(),
            pose_orientation.z();
        Qm.col(X_idx) = Q;
      }

      Eigen::EigenSolver<Eigen::MatrixXf> solver(Qm * Qm.transpose());

      auto eigenvectors = solver.eigenvectors();
      auto eigenvalues = solver.eigenvalues();

      if (eigenvalues.rows() < 4)
        throw gmCore::RuntimeException(
            "Could not find average orientation from samples");

      Eigen::Vector4f V = eigenvectors.col(0).real();
      float best_value = eigenvalues(0, 0).real();
      for (int idx = 1; idx < eigenvalues.rows(); ++idx) {
        if (eigenvalues(idx, 0).real() < best_value) continue;
        V = eigenvectors.col(idx).real();
        best_value = eigenvalues(idx, 0).real();
      }

      Eigen::Quaternionf Q = Eigen::Quaternionf(V[0], V[1], V[2], V[3]);
      res.poses.push_back({P, Q});
    }

    return res;
  };

  solver.func_reflect = [](const TPoseList &Xm, const TPoseList Xn) -> TPoseList {
    TPoseList res;
    res.poses.reserve(Xm.poses.size());
    for (size_t idx = 0; idx < Xm.poses.size(); ++idx) {
      const auto &Pm = Xm.poses[idx];
      const auto &Pn = Xn.poses[idx];
      res.poses.push_back(
          {Pm.position * 2.f - Pn.position,
           Pm.orientation * (Pm.orientation * Pn.orientation.conjugate())});
    }

    return res; // Xm * 2.f - Xn;
  };

  solver.func_mean = [](const TPoseList &XA, const TPoseList XB) -> TPoseList {
    TPoseList res;
    res.poses.reserve(XA.poses.size());
    for (size_t idx = 0; idx < XA.poses.size(); ++idx) {
      const auto &PA = XA.poses[idx];
      const auto &PB = XB.poses[idx];
      res.poses.push_back({PA.position * .5f + PB.position * .5f,
                           PA.orientation.slerp(.5f, PB.orientation)});
    }
    return res; // XA * 0.5f + XB * 0.5f;
  };

  size_t iterations = 0;
  auto res = solver.solve(X0, iterations);
#else
  TPoseList res;
  res.poses.reserve(absolute_poses.size());
  for (size_t idx = 0; idx < absolute_poses.size(); ++idx)
    res.poses.push_back(absolute_poses[idx]);
#endif

  for (size_t idx = 0; idx < res.poses.size(); ++idx)
    relative_registration.value()[base_indices[idx]] = res.poses[idx];
}

void RelativeTrackerPoseRegistrationEstimator::addSinglePoseTracker(
    std::shared_ptr<SinglePoseTracker> tracker) {
  _impl->single_pose_trackers.push_back(tracker);
}

void RelativeTrackerPoseRegistrationEstimator::addMultiPoseTracker(
    std::shared_ptr<MultiPoseTracker> tracker) {
  _impl->multi_pose_trackers.push_back(tracker);
}

void RelativeTrackerPoseRegistrationEstimator::setSamplesPerSecond(float n) {
  _impl->sample_delay = gmCore::TimeTools::secondsToDuration(1.f / n);
}

void RelativeTrackerPoseRegistrationEstimator::setWarningThreshold(float d) {
  _impl->warning_threshold = d;
}

void RelativeTrackerPoseRegistrationEstimator::setInlierThreshold(float d) {
  _impl->inlier_threshold = d;
}

void RelativeTrackerPoseRegistrationEstimator::setOrientationWarningThreshold(float d) {
  _impl->orientation_warning_threshold = d;
}

void RelativeTrackerPoseRegistrationEstimator::setOrientationInlierThreshold(float d) {
  _impl->orientation_inlier_threshold = d;
}

size_t
RelativeTrackerPoseRegistrationEstimator::getSinglePoseTrackerCount() const {
  return _impl->single_pose_trackers.size();
}

RelativeTrackerPoseRegistrationEstimator::Pose
RelativeTrackerPoseRegistrationEstimator::getSinglePoseTrackerPose(
    size_t idx) const {
  if (!_impl->relative_registration.has_value())
    const_cast<RelativeTrackerPoseRegistrationEstimator *>(this)
        ->_impl->estimateRegistration();
  return (*_impl->relative_registration)[idx_t::single(idx)];
}

size_t
RelativeTrackerPoseRegistrationEstimator::getMultiPoseTrackerCount() const {
  return _impl->multi_pose_trackers.size();
}

std::map<int, RelativeTrackerPoseRegistrationEstimator::Pose>
RelativeTrackerPoseRegistrationEstimator::getMultiPoseTrackerPose(
    size_t idx) const {
  if (!_impl->relative_registration.has_value())
    const_cast<RelativeTrackerPoseRegistrationEstimator *>(this)
        ->_impl->estimateRegistration();

  std::map<int, Pose> res;
  for (const auto &pose : *_impl->relative_registration) {
    if (pose.first.tracker != idx) continue;
    res[pose.first.item] = pose.second;
  }
  return res;
}

void RelativeTrackerPoseRegistrationEstimator::traverse(Visitor *visitor) {
  for (auto tracker : _impl->single_pose_trackers) tracker->accept(visitor);
  for (auto tracker : _impl->multi_pose_trackers) tracker->accept(visitor);
}

END_NAMESPACE_GMTRACK;
