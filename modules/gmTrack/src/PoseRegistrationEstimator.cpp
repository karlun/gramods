
#include <gmTrack/PoseRegistrationEstimator.hh>

#include <gmTrack/ButtonsMapper.hh>

#include <gmCore/RunOnce.hh>
#include <gmCore/Console.hh>

#include <Eigen/LU>

#include <limits>
#include <type_traits>

BEGIN_NAMESPACE_GMTRACK;

GM_OFI_DEFINE(PoseRegistrationEstimator);
GM_OFI_PARAM(PoseRegistrationEstimator, samplesPerSecond, float, PoseRegistrationEstimator::setSamplesPerSecond);
GM_OFI_PARAM(PoseRegistrationEstimator, point, Eigen::Vector3f, PoseRegistrationEstimator::addPoint);
GM_OFI_PARAM(PoseRegistrationEstimator, trackerPoint, Eigen::Vector3f, PoseRegistrationEstimator::addTrackerPoint);
GM_OFI_POINTER(PoseRegistrationEstimator, controller, Controller, PoseRegistrationEstimator::setController);

struct PoseRegistrationEstimator::Impl {

  typedef gmCore::Updateable::clock clock;

  std::vector<Eigen::Vector3f> actual_positions;
  std::vector<Eigen::Vector3f> tracker_positions;
  std::vector<Eigen::Vector3f> samples;
  clock::time_point last_sample_time = clock::time_point::min();
  float samples_per_second = 1;
  bool collecting = false;

  void update(clock::time_point t);

  void getIQM3D(std::vector<Eigen::Vector3f> samples, Eigen::Vector3f &x);
  float estimateSphericity(std::vector<Eigen::Vector3f> samples);

  void performRegistration();

  /**
     Identifies the degenerate dimension and copies the data into two
     sets distributed in this dimension.

     @param idx0 If larger than or equal to zero, specifies the index
     of the first data point that should be used to identify the
     orientation of the data, otherwise set to that index.

     @param idx1 Specifies the index of the second data point that
     should be used to identify the orientation of the data,
     alternatively set to that index.
  */
  void expandPlanar(std::vector<Eigen::Vector3f> &data, int &idx0, int &idx1);

  bool estimateRegistration(std::vector<Eigen::Vector3f> tracker_data,
                            std::vector<Eigen::Vector3f> actual_data,
                            Eigen::Matrix4f &M);

  void estimateUnitRegistration(std::vector<Eigen::Vector3f> tracker_data,
                                std::vector<Eigen::Vector3f> actual_data,
                                Eigen::Matrix4f M_raw,
                                Eigen::Matrix4f &M_unit);

  float planar_sphericity = 0.3f;
  std::shared_ptr<gramods::gmTrack::Controller> controller;
  gramods::gmTrack::ButtonsTracker::ButtonsSample buttons;

  Eigen::Matrix4f registration_raw;
  Eigen::Matrix4f registration_unit;
  bool successful_registration = false;
};

PoseRegistrationEstimator::PoseRegistrationEstimator()
  : Updateable(-1000),
    _impl(std::make_unique<Impl>()) {}

PoseRegistrationEstimator::~PoseRegistrationEstimator() {}

void PoseRegistrationEstimator::update(clock::time_point t) {
  _impl->update(t);
}

void PoseRegistrationEstimator::setController(std::shared_ptr<gramods::gmTrack::Controller> controller) {
  _impl->controller = controller;
}

void PoseRegistrationEstimator::addPoint(Eigen::Vector3f p) {
  _impl->actual_positions.push_back(p);
}

void PoseRegistrationEstimator::addTrackerPoint(Eigen::Vector3f p) {
  _impl->tracker_positions.push_back(p);
}

void PoseRegistrationEstimator::setSamplesPerSecond(float n) {
  _impl->samples_per_second = n;
}

bool PoseRegistrationEstimator::getRegistration(Eigen::Matrix4f * RAW, Eigen::Matrix4f * UNIT) {

  if (!_impl->successful_registration)
    return false;

  if (RAW)
    *RAW = _impl->registration_raw;

  if (UNIT)
    *UNIT = _impl->registration_unit;

  return true;
}

void PoseRegistrationEstimator::Impl::update(clock::time_point now) {

  if (!tracker_positions.empty() &&
      tracker_positions.size() == actual_positions.size()) {
    performRegistration();
    return;
  }

  if (!controller) {
    GM_RUNONCE(GM_ERR("PoseRegistrationEstimator", "No controller to calibrate"));
    return;
  }

  controller->getButtons(buttons);

  if (!collecting) {
    if (buttons.buttons[ButtonsMapper::ButtonIdx::MAIN]) {
      collecting = true;
      GM_INF("PoseRegistrationEstimator", "going into collect mode");
    } else {
      return;
    }
  }

  if (buttons.buttons[ButtonsMapper::ButtonIdx::MAIN]){
    gramods::gmTrack::PoseTracker::PoseSample pose;
    if (! controller->getPose(pose)) {
      GM_RUNONCE(GM_ERR("PoseRegistrationEstimator", "Cannot read controller pose"));
      return;
    }

    // Zero or less samples per second results in taking only one sample per click
    if (samples_per_second < std::numeric_limits<float>::epsilon()) {
      if (last_sample_time == clock::time_point::min()) {
        GM_DBG1("PoseRegistrationEstimator", "collecting a single sample");
        samples.push_back(pose.position);
      }
      last_sample_time = now;
      return;
    }

    // Do not take samples faster than samples_per_second
    if (last_sample_time != clock::time_point::min() &&
        (now - last_sample_time) < std::chrono::milliseconds(int(1000.f/samples_per_second)))
      return;

    GM_DBG1("PoseRegistrationEstimator", "collecting sample");
    samples.push_back(pose.position);
    last_sample_time = now;

    return;
  }

  // Button released - finished collecting samples

  collecting = false;
  last_sample_time = clock::time_point::min();

  assert(!samples.empty());

  Eigen::Vector3f pos;
  getIQM3D(samples, pos);
  tracker_positions.push_back(pos);

  GM_INF("PoseRegistrationEstimator", "left collect mode (have " << tracker_positions.size() << " of " << actual_positions.size() << ")");
  GM_DBG1("PoseRegistrationEstimator", "estimated tracked position (" << pos.transpose() << ") out of " << samples.size() << " samples, for actual position (" << actual_positions[tracker_positions.size() - 1].transpose() << ")");

  samples.clear();

  if (tracker_positions.size() < actual_positions.size())
    return;

  GM_INF("PoseRegistrationEstimator", "have all " << actual_positions.size() << " samples");
  performRegistration();
}

void PoseRegistrationEstimator::Impl::performRegistration() {

  float tracker_data_sph = estimateSphericity(tracker_positions);
  float actual_data_sph = estimateSphericity(actual_positions);

  if ((tracker_data_sph <= planar_sphericity) && (actual_data_sph > planar_sphericity)) {
    GM_WRN("PoseRegistrationEstimator", "sphericity inconsistency - tracker positions are in a plane but not actual positions");
  } else if ((tracker_data_sph > planar_sphericity) && (actual_data_sph <= planar_sphericity)) {
    GM_WRN("PoseRegistrationEstimator", "sphericity inconsistency - actual positions are planar but not tracker positions");
  }

  std::vector<Eigen::Vector3f> tracker_data = tracker_positions;
  std::vector<Eigen::Vector3f> actual_data = actual_positions;

  if (std::min(tracker_data_sph, actual_data_sph) <= planar_sphericity) {

    GM_WRN("PoseRegistrationEstimator", "Samples will be manipulated to compensate for poor sphericity(" << tracker_data_sph << " and " << actual_data_sph << " for tracker data and actual data, respectively)");

    int idx0 = -1, idx1;
    expandPlanar(actual_data, idx0, idx1);
    expandPlanar(tracker_data, idx0, idx1);
  }

  Eigen::Matrix4f M_reg;
  if (!estimateRegistration(tracker_data, actual_data, M_reg))
    return;

  registration_raw = M_reg;
  successful_registration = true;

  GM_DBG1("PoseRegistrationEstimator", "Raw registration matrix:\n" << M_reg);

  Eigen::Matrix4f M_unit;
  estimateUnitRegistration(tracker_data, actual_data, M_reg, M_unit);
  registration_unit = M_unit;

  GM_DBG1("PoseRegistrationEstimator", "Unit registration matrix:\n" << M_unit);

  tracker_positions.clear();
}

void PoseRegistrationEstimator::Impl::getIQM3D(std::vector<Eigen::Vector3f> samples, Eigen::Vector3f &x) {

  assert(!samples.empty());

  if (samples.size() == 1) {
    x = samples.front();
    return;
  }

  if (samples.size() < 4) {
    Eigen::Vector3f sum;
    for (auto p : samples) sum += p;
    x = (1.0 / samples.size()) * sum;
    return;
  }

  Eigen::Vector3f sum = Eigen::Vector3f::Zero();

  std::sort(samples.begin(), samples.end(),
            [](Eigen::Vector3f a, Eigen::Vector3f b){
              return a[0] < b[0];
            });
  for (size_t idx = samples.size()/4; idx < (3*samples.size())/4; ++idx)
    sum[0] += samples[idx][0];

  std::sort(samples.begin(), samples.end(),
            [](Eigen::Vector3f a, Eigen::Vector3f b){
              return a[1] < b[1];
            });
  for (size_t idx = samples.size()/4; idx < (3*samples.size())/4; ++idx)
    sum[1] += samples[idx][1];

  std::sort(samples.begin(), samples.end(),
            [](Eigen::Vector3f a, Eigen::Vector3f b){
              return a[2] < b[2];
            });
  for (size_t idx = samples.size()/4; idx < (3*samples.size())/4; ++idx)
    sum[2] += samples[idx][2];

  x = (1.0 / ((3*samples.size())/4 - samples.size()/4)) * sum;

}


float PoseRegistrationEstimator::Impl::estimateSphericity(std::vector<Eigen::Vector3f> data) {

  Eigen::Vector3f cp = Eigen::Vector3f::Zero();
  for (auto pt : data)
    cp += pt;
  cp /= (float)data.size();

  Eigen::MatrixXf data_matrix(3, data.size());

  for (size_t idx = 0; idx < data.size(); ++idx) {
    data_matrix.col(idx) = (data[idx] - cp);
  }

  Eigen::JacobiSVD<Eigen::MatrixXf> svd(data_matrix, Eigen::ComputeThinU | Eigen::ComputeThinV);
  auto singular_values = svd.singularValues();
  assert(singular_values.rows() == 3);

  GM_DBG1("PoseRegistrationEstimator", "data matrix:\n" << data_matrix);
  GM_DBG1("PoseRegistrationEstimator", "singular values: " << singular_values.transpose());

  if (singular_values[1] <= std::numeric_limits<std::remove_reference<decltype(singular_values[1])>::type>::epsilon()) {
    GM_ERR("PoseRegistrationEstimator", "Points are too linearly dependent for any further processing");
    throw std::runtime_error("Points are too linearly dependent for any further processing");
  } else if (singular_values[1] / singular_values[0] < 0.3f) {
    // Arbitrarily choosen sphericity limit for warning only
    GM_WRN("PoseRegistrationEstimator", "Poor second axis sphericity (" << (singular_values[1] / singular_values[0]) << ") - points may be too linearly dependent for a good registration estimation");
  }

  return singular_values[2] / singular_values[0];
}


void PoseRegistrationEstimator::Impl::expandPlanar(std::vector<Eigen::Vector3f> &data,
                                                   int &idx0, int &idx1) {
  assert(data.size() >= 3);

  Eigen::Vector3f cp = Eigen::Vector3f::Zero();
  for (auto pt : data)
    cp += pt;
  cp /= (float)data.size();

  Eigen::MatrixXf data_matrix(3, data.size());

  for (size_t idx = 0; idx < data.size(); ++idx) {
    data_matrix.col(idx) = (data[idx] - cp);
  }

  Eigen::JacobiSVD<Eigen::MatrixXf> svd(data_matrix, Eigen::ComputeFullU);
  auto U = svd.matrixU();
  auto S = svd.singularValues();

  Eigen::Vector3f data_X = U.col(0);
  Eigen::Vector3f data_Y = U.col(1);
  Eigen::Vector3f data_normal = U.col(2);
  float data_scale = S[0];
  GM_DBG1("PoseRegistrationEstimator", "Estimated data normal: " << data_normal.transpose());
  GM_DBG1("PoseRegistrationEstimator", "Estimated data scale: " << data_scale);

  if (idx0 < 0) {

    float best_value0 = 0.f;
    for (size_t idx = 0; idx < data.size(); ++idx) {
      float value = fabsf(data_X.dot((data[idx] - cp)));
      if (value > best_value0) {
        best_value0 = value;
        idx0 = idx;
      }
    }

    float best_value1 = 0.f;
    for (size_t idx = 0; idx < data.size(); ++idx) {
      if (idx == (size_t)idx0) continue;
      float value = fabsf(data_Y.dot((data[idx] - cp)));
      if (value > best_value1) {
        best_value1 = value;
        idx1 = idx;
      }
    }

    GM_DBG1("PoseRegistrationEstimator", "Estimated primary samples: " << idx0 << " (" << best_value0 << ") and " << idx1 << "(" << best_value1 << ")");
  }

  if ((data[idx0] - cp).cross(data[idx1] - cp).dot(data_normal) < 0) {
    GM_DBG1("PoseRegistrationEstimator", "Flipping");
    data_normal = -data_normal;
  }

  std::vector<Eigen::Vector3f> new_data;
  new_data.reserve(2 * data.size());

  Eigen::Vector3f offset = (0.5f * data_scale) * data_normal;
  for (auto pt : data)
    new_data.push_back(pt + offset);
  for (auto pt : data)
    new_data.push_back(pt - offset);

  GM_DBG3("PoseRegistrationEstimator", "New data:");
  for (auto pt : new_data)
    GM_DBG3("PoseRegistrationEstimator", "" << pt.transpose());

  data.swap(new_data);
}

bool PoseRegistrationEstimator::Impl::estimateRegistration
(std::vector<Eigen::Vector3f> tracker_data,
 std::vector<Eigen::Vector3f> actual_data,
 Eigen::Matrix4f &M) {

  assert(tracker_data.size() == actual_data.size());
  size_t N = tracker_data.size();

  Eigen::MatrixXf tracker_set(4, N);
  Eigen::MatrixXf actual_set(4, N);

  for (size_t idx = 0; idx < N; ++idx) {
    tracker_set.col(idx) = tracker_data[idx].homogeneous();
    actual_set.col(idx) = actual_data[idx].homogeneous();
  }

  if (N == 4) {

    GM_DBG1("PoseRegistrationEstimator", "Solving complete system by inverse multiplication");

    // ACTUAL = M_REG  TRACKER
    // M_REG = ACTUAL / TRACKER
    // M_REG = ACTUAL  TRACKER^-1

    Eigen::Matrix4f tracker_set_4f = tracker_set;
    Eigen::Matrix4f tracker_set_inverse = tracker_set_4f.inverse();

    M = actual_set * tracker_set_inverse;

  } else {

    GM_DBG1("PoseRegistrationEstimator", "Solving overdetermined system by inverse multiplication");

    // M_REG  TRACKER = ACTUAL
    // TRACKER^t M_REG^t = ACTUAL^t
    // Ax = b

    auto A = tracker_set.transpose();
    auto b = actual_set.transpose();

    auto qr = A.fullPivHouseholderQr();
    auto x = qr.solve(b);
    M = x.transpose();
  }

  GM_DBG1("PoseRegistrationEstimator", "Raw registration error: " << ((M * tracker_set) - actual_set).norm());

  return true;
}


void PoseRegistrationEstimator::Impl::estimateUnitRegistration
(std::vector<Eigen::Vector3f> tracker_data,
 std::vector<Eigen::Vector3f> actual_data,
 Eigen::Matrix4f M_raw,
 Eigen::Matrix4f &M_unit) {

  if (tracker_data.empty() || actual_data.empty())
    throw gmCore::InvalidArgument("empty vector of data not supported");

  if (tracker_data.size() != actual_data.size())
    throw gmCore::InvalidArgument("tracker and actual point vectors must be of equal size");

  Eigen::JacobiSVD<Eigen::MatrixXf> svd(M_raw.block<3,3>(0,0),
                                        Eigen::ComputeFullU | Eigen::ComputeFullV);
  auto U = svd.matrixU();
  auto S = svd.singularValues();
  auto V = svd.matrixV();

  M_unit = Eigen::Matrix4f::Identity();
  M_unit.block<3,3>(0,0) = U * V.transpose();

  Eigen::Vector3f tracker_cp = Eigen::Vector3f::Zero();
  for (auto pt : tracker_data)
    tracker_cp += pt;
  tracker_cp /= (float)tracker_data.size();

  Eigen::Vector3f actual_cp = Eigen::Vector3f::Zero();
  for (auto pt : actual_data)
    actual_cp += pt;
  actual_cp /= (float)actual_data.size();

  Eigen::Vector3f offset = actual_cp - (M_unit * tracker_cp.homogeneous()).hnormalized();
  M_unit.block<3,1>(0,3) = offset;
}

END_NAMESPACE_GMTRACK;
