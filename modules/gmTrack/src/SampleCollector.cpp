
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
GM_OFI_PARAM(SampleCollector, trackerPosition, Eigen::Vector3f, SampleCollector::addTrackerPosition);
GM_OFI_PARAM(SampleCollector, trackerOrientation, Eigen::Quaternionf, SampleCollector::addTrackerOrientation);
GM_OFI_POINTER(SampleCollector, controller, Controller, SampleCollector::setController);


SampleCollector::SampleCollector(Impl *_impl)
  : Updateable(-1000), _impl(_impl) {
  if (!this->_impl) this->_impl = std::make_unique<Impl>();
}

SampleCollector::~SampleCollector() {}

void SampleCollector::update(clock::time_point t) {
  _impl->update(t);
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

void SampleCollector::Impl::update(clock::time_point now) {

  if (!controller && tracker_positions.empty() && tracker_orientations.empty()) {
    GM_RUNONCE(GM_ERR("SampleCollector", "No tracker positions or orientations specified and no controller to read tracker data from"));
    return;
  }

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

  Eigen::Vector3f pos;
  getAverage(sample_positions, pos);
  tracker_positions.push_back(pos);
  sample_positions.clear();

  Eigen::Quaternionf ori;
  getAverage(sample_orientations, ori);
  tracker_orientations.push_back(ori);
  sample_orientations.clear();
}

void SampleCollector::Impl::getAverage(std::vector<Eigen::Vector3f> samples,
                                       Eigen::Vector3f &x) {

  if (samples.empty())
    throw gmCore::InvalidArgument("Cannot average empty vector");

  if (samples.size() == 1) {
    x = samples.front();
    return;
  }

  Eigen::Vector3f sum = Eigen::Vector3f::Zero();
  for (auto p : samples) sum += p;
  x = (1.0 / samples.size()) * sum;

  float stddev = 0.f;
  for (auto p : samples) stddev += (p - x).squaredNorm();
  stddev = std::sqrt(stddev);

  float worst_sqr_offset = 0.f;
  for (size_t idx = 0; idx < samples.size(); ++idx) {
    float sqr_offset = (samples[idx] - x).squaredNorm();
    worst_sqr_offset = std::max(worst_sqr_offset, sqr_offset);
  }
  float worst_offset = std::sqrt(worst_sqr_offset);

  if (worst_offset > warning_threshold) {
    GM_WRN("SampleCollector",
           "Estimated mean " << x.transpose() << " (stddev " << stddev
                             << ") has worst offset " << worst_offset << " in "
                             << samples.size() << " samples.");
  } else {
    GM_DBG1("SampleCollector",
            "Estimated mean " << x.transpose() << " (stddev " << stddev
                              << ", worst offset " << worst_offset << ") from "
                              << samples.size() << " samples.");
  }
}

void SampleCollector::Impl::getAverage(std::vector<Eigen::Quaternionf> samples,
                                       Eigen::Quaternionf &x) {

  if (samples.empty())
    throw gmCore::InvalidArgument("Cannot average empty vector");

  if (samples.size() == 1) {
    x = samples.front();
    return;
  }

  float s = 1.f / samples.size();

  Eigen::MatrixXf Qm(4, samples.size());
  for (size_t idx = 0; idx < samples.size(); ++idx) {
    Eigen::MatrixXf Q(4,1);
    Q << samples[idx].w(), samples[idx].x(), samples[idx].y(), samples[idx].z();
    Qm.col(idx) = s * Q;
  }

  Eigen::EigenSolver<Eigen::MatrixXf> solver(Qm * Qm.transpose());

  auto eigenvectors = solver.eigenvectors();
  auto eigenvalues = solver.eigenvalues();

  if (eigenvalues.rows() < 4)
    throw gmCore::RuntimeException("Could not find average orientation from samples");

  float best_value = std::numeric_limits<float>::min();
  Eigen::Vector4f V;
  for (int idx = 0; idx < eigenvalues.rows(); ++idx) {
    if (eigenvalues(idx, 0).real() < best_value) continue;
    V = eigenvectors.col(idx).real();
  }

  x = Eigen::Quaternionf(V[0], V[1], V[2], V[3]);
}

const std::vector<Eigen::Vector3f> &
SampleCollector::getTrackerPositions() const {
  return _impl->tracker_positions;
}

const std::vector<Eigen::Quaternionf> &
SampleCollector::getTrackerOrientations() const {
  return _impl->tracker_orientations;
}

END_NAMESPACE_GMTRACK;