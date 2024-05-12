
#include <gmTrack/SampleCollector.hh>
#include <gmTrack/SampleCollector.impl.hh>

#include <gmTrack/ButtonsMapper.hh>

#include <gmCore/RunOnce.hh>
#include <gmCore/Console.hh>

#include <Eigen/LU>

#include <limits>
#include <type_traits>

BEGIN_NAMESPACE_GMTRACK;

GM_OFI_DEFINE(SampleCollector);
GM_OFI_PARAM(SampleCollector, samplesPerSecond, float, SampleCollector::setSamplesPerSecond);
GM_OFI_PARAM(SampleCollector, warningThreshold, float, SampleCollector::setWarningThreshold);
GM_OFI_PARAM(SampleCollector, orientationWarningThreshold, float, SampleCollector::setOrientationWarningThreshold);
GM_OFI_PARAM(SampleCollector, trackerPosition, Eigen::Vector3f, SampleCollector::addTrackerPosition);
GM_OFI_PARAM(SampleCollector, trackerOrientation, Eigen::Quaternionf, SampleCollector::addTrackerOrientation);
GM_OFI_POINTER(SampleCollector, controller, Controller, SampleCollector::setController);


SampleCollector::SampleCollector(Impl *_impl)
  : Updateable(-1000), _impl(_impl) {
  if (!this->_impl) this->_impl = std::make_unique<Impl>();
}

SampleCollector::~SampleCollector() {}

void SampleCollector::update(clock::time_point time, size_t frame) {
  _impl->update(time);
}

void SampleCollector::setController(std::shared_ptr<gramods::gmTrack::Controller> controller) {
  _impl->controller = controller;
}

void SampleCollector::addTrackerPosition(Eigen::Vector3f p) {
  _impl->tracker_positions.push_back(p);
}

void SampleCollector::addTrackerOrientation(Eigen::Quaternionf o) {
  _impl->tracker_orientations.push_back(o);
}

void SampleCollector::setSamplesPerSecond(float n) {
  _impl->samples_per_second = n;
}

void SampleCollector::setWarningThreshold(float d) {
  _impl->warning_threshold = d;
}

void SampleCollector::setInlierThreshold(float d) {
  _impl->inlier_threshold = d;
}

void SampleCollector::setOrientationWarningThreshold(float d) {
  _impl->orientation_warning_threshold = d;
}

void SampleCollector::setOrientationInlierThreshold(float d) {
  _impl->orientation_inlier_threshold = d;
}

void SampleCollector::Impl::update(clock::time_point now) {

  if (!controller && tracker_positions.empty() && tracker_orientations.empty()) {
    GM_RUNONCE(GM_ERR("SampleCollector", "No tracker positions or orientations specified and no controller to read tracker data from"));
    return;
  }

  if (!controller) return;

  gramods::gmTrack::ButtonsTracker::ButtonsSample buttons;
  controller->getButtons(buttons);

  if (!collecting) {
    if (buttons.buttons[ButtonsMapper::ButtonIdx::MAIN]) {
      collecting = true;
      GM_INF("SampleCollector", "going into collect mode");
    } else {
      return;
    }
  }

  if (buttons.buttons[ButtonsMapper::ButtonIdx::MAIN]){
    gramods::gmTrack::PoseTracker::PoseSample pose;
    if (! controller->getPose(pose)) {
      GM_RUNONCE(GM_ERR("SampleCollector", "Cannot read controller pose"));
      return;
    }

    // Zero or less samples per second results in taking only one sample per click
    if (samples_per_second < std::numeric_limits<float>::epsilon()) {
      if (last_sample_time == clock::time_point::min()) {
        GM_DBG1("SampleCollector", "collecting a single sample");
        sample_positions.push_back(pose.position);
        sample_orientations.push_back(pose.orientation);
      }
      last_sample_time = now;
      return;
    }

    // Do not take samples faster than samples_per_second
    if (last_sample_time != clock::time_point::min() &&
        (now - last_sample_time) < std::chrono::milliseconds(int(1000.f/samples_per_second)))
      return;

    GM_DBG1("SampleCollector", "collecting sample");
    sample_positions.push_back(pose.position);
    sample_orientations.push_back(pose.orientation);
    last_sample_time = now;

    return;
  }

  // Button released - finished collecting samples

  collecting = false;
  last_sample_time = clock::time_point::min();

  if (sample_positions.empty()) {
    GM_RUNONCE(GM_ERR("SampleCollector", "No samples collected"));
    return;
  }

  float stddev, maxdev;
  size_t inlier_count;
  Eigen::Vector3f pos = getAverage(
      sample_positions, &stddev, &maxdev, inlier_threshold, &inlier_count);
  tracker_positions.push_back(pos);

  if (maxdev > warning_threshold) {
    GM_WRN("SampleCollector",
           "Estimated mean, " << pos.transpose() << " (stddev " << stddev
                              << "), has worst offset " << maxdev << " in "
                              << inlier_count << " of "
                              << sample_positions.size() << " samples.");
  } else {
    GM_INF("SampleCollector",
           "Estimated mean: " << pos.transpose() << " (stddev " << stddev
                              << ", worst offset " << maxdev << ") from "
                              << inlier_count << " of "
                              << sample_positions.size() << " samples.");
  }
  sample_positions.clear();

  Eigen::Quaternionf ori = getAverage(sample_orientations,
                                      &stddev,
                                      &maxdev,
                                      orientation_inlier_threshold,
                                      &inlier_count);
  tracker_orientations.push_back(ori);

  if (maxdev > orientation_warning_threshold) {
    GM_WRN("SampleCollector",
           "Estimated orientation (stddev "
               << stddev << "), has worst offset " << maxdev << " in "
               << inlier_count << " of " << sample_orientations.size()
               << " samples.");
  } else {
    GM_INF("SampleCollector",
           "Estimated orientation (stddev "
               << stddev << ", worst offset " << maxdev << ") from "
               << inlier_count << " of " << sample_orientations.size()
               << " samples.");
  }
  sample_orientations.clear();
}

Eigen::Vector3f
SampleCollector::getAverage(std::vector<Eigen::Vector3f> samples,
                            float *stddev,
                            float *maxdev,
                            float inlier_dist,
                            size_t *inlier_count) {

  if (samples.empty())
    throw gmCore::InvalidArgument("Cannot average empty vector");

  if (samples.size() == 1) {
    if (stddev) *stddev = 0.f;
    if (maxdev) *maxdev = 0.f;
    if (inlier_count) *inlier_count = samples.size();
    return samples.front();
  }

  Eigen::Vector3f x;

  if (inlier_dist == std::numeric_limits<float>::max()) {
    x = Eigen::Vector3f::Zero();
    for (auto p : samples) x += p;
    x *= (1.0 / samples.size());
  } else {

    float inl_d2 = inlier_dist * inlier_dist;
    while (true) {
      x = Eigen::Vector3f::Zero();
      for (auto p : samples) x += p;
      x *= (1.0 / samples.size());

      if (samples.size() == 1) break;

      size_t worst_idx = 0;
      float worst_distance = (samples[worst_idx] - x).squaredNorm();
      for (size_t idx = 1; idx < samples.size(); ++idx) {
        float distance = (samples[idx] - x).squaredNorm();
        if (distance < worst_distance) continue;

        worst_idx = idx;
        worst_distance = distance;
      }

      if (worst_distance < inl_d2) break;

      GM_DBG2("SampleCollector",
              "dropped positional outlier for position average ("
                  << std::sqrt(worst_distance) << " > " << inlier_dist << " @ "
                  << samples[worst_idx].transpose() << ")");
      samples.erase(samples.begin() + worst_idx);
    }
  }

  if (stddev) {
    float dev = 0.f;
    for (auto p : samples) dev += (p - x).squaredNorm();
    *stddev = std::sqrt(dev);
  }

  if (maxdev) {
    float dev = 0.f;
    for (auto p : samples) dev = std::max(dev, (p - x).squaredNorm());
    *maxdev = std::sqrt(dev);
  }

  if (inlier_count) *inlier_count = samples.size();

  return x;
}

Eigen::Quaternionf
SampleCollector::getAverage(std::vector<Eigen::Quaternionf> samples,
                            float *stddev,
                            float *maxdev,
                            float inlier_dist,
                            size_t *inlier_count) {

  if (samples.empty())
    throw gmCore::InvalidArgument("Cannot average empty vector");

  if (samples.size() == 1) {
    if (stddev) *stddev = 0.f;
    if (maxdev) *maxdev = 0.f;
    if (inlier_count) *inlier_count = samples.size();
    return samples.front();
  }

  Eigen::Quaternionf x;
  while (true) {
    float s = 1.f / samples.size();

    Eigen::MatrixXf Qm(4, samples.size());
    for (size_t idx = 0; idx < samples.size(); ++idx) {
      Eigen::MatrixXf Q(4, 1);
      Q << samples[idx].w(), samples[idx].x(), samples[idx].y(),
          samples[idx].z();
      Qm.col(idx) = s * Q;
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

    x = Eigen::Quaternionf(V[0], V[1], V[2], V[3]);

    if (inlier_dist <= 0.f) break;
    if (samples.size() == 1) break;

    size_t worst_idx = 0;
    float worst_angle =
        Eigen::AngleAxisf(samples[worst_idx].conjugate() * x).angle();
    for (size_t idx = 1; idx < samples.size(); ++idx) {
      float angle = Eigen::AngleAxisf(samples[idx].conjugate() * x).angle();
      if (angle < worst_angle) continue;

      worst_idx = idx;
      worst_angle = angle;
    }

    if (worst_angle < inlier_dist) break;

    GM_DBG2("SampleCollector",
            "dropped orientational outlier for position average ("
                << worst_angle << " > " << inlier_dist << " @ "
                << Eigen::AngleAxisf(samples[worst_idx]).axis().transpose()
                << ", " << Eigen::AngleAxisf(samples[worst_idx]).angle()
                << ")");
    samples.erase(samples.begin() + worst_idx);
  }

  if (stddev) {
    float dev = 0.f;
    for (const auto sample : samples) {
      float a = Eigen::AngleAxisf(sample.conjugate() * x).angle();
      dev += a * a;
    }
    *stddev = std::sqrt(dev);
  }

  if (maxdev) {
    float dev = 0.f;
    for (const auto sample : samples) {
      float a = Eigen::AngleAxisf(sample.conjugate() * x).angle();
      dev = std::max(dev, std::abs(a));
    }
    *maxdev = dev;
  }

  if (inlier_count) *inlier_count = samples.size();

  return x;
}

const std::vector<Eigen::Vector3f> &
SampleCollector::getTrackerPositions() const {
  return _impl->tracker_positions;
}

const std::vector<Eigen::Quaternionf> &
SampleCollector::getTrackerOrientations() const {
  return _impl->tracker_orientations;
}

void SampleCollector::traverse(Visitor *visitor) {
  if (_impl->controller) _impl->controller->accept(visitor);
}

END_NAMESPACE_GMTRACK;
