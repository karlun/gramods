
#include <gmTrack/BaseEstimator.hh>

#include <Eigen/LU>

BEGIN_NAMESPACE_GMTRACK;

GM_OFI_DEFINE(BaseEstimator);
GM_OFI_PARAM(BaseEstimator, samplesPerSecond, float, BaseEstimator::setSamplesPerSecond);
GM_OFI_PARAM(BaseEstimator, point, gmTypes::float3, BaseEstimator::addPoint);
GM_OFI_POINTER(BaseEstimator, controller, Controller, BaseEstimator::setController);

struct BaseEstimator::Impl {
  Impl();

  typedef gmCore::Updateable::clock clock;

  std::vector<Eigen::Vector3f> actual_positions;
  std::vector<Eigen::Vector3f> tracker_positions;
  std::vector<Eigen::Vector3f> samples;
  clock::time_point last_sample_time;
  float samples_per_second;
  bool collecting = false;

  void update(clock::time_point t);

  void getIQM3D(std::vector<Eigen::Vector3f> samples, Eigen::Vector3f &x);
  float estimateSphericity(std::vector<Eigen::Vector3f> samples);

  void expandPlanar(std::vector<Eigen::Vector3f> &data);

  bool estimateRegistration(std::vector<Eigen::Vector3f> tracker_data,
                            std::vector<Eigen::Vector3f> actual_data,
                            Eigen::Matrix4f &M);

  void estimateUnitRegistration(std::vector<Eigen::Vector3f> tracker_data,
                                std::vector<Eigen::Vector3f> actual_data,
                                Eigen::Matrix4f M_raw,
                                Eigen::Matrix4f &M_unit);

  float planar_sphericity;
  std::shared_ptr<gramods::gmTrack::Controller> controller;

  Eigen::Matrix4f registration_raw;
  Eigen::Matrix4f registration_unit;
  bool successful_registration = false;
};

BaseEstimator::BaseEstimator()
  : _impl(std::make_unique<Impl>()) {}

BaseEstimator::Impl::Impl()
  : samples_per_second(1),
    last_sample_time(clock::time_point::min()),
    planar_sphericity(0.3) {}

void BaseEstimator::update(clock::time_point t) {
  _impl->update(t);
}

void BaseEstimator::setController(std::shared_ptr<gramods::gmTrack::Controller> controller) {
  _impl->controller = controller;
}

void BaseEstimator::addPoint(gmTypes::float3 p) {
  _impl->actual_positions.push_back(Eigen::Vector3f(p[0], p[1], p[2]));
}

void BaseEstimator::setSamplesPerSecond(float n) {
  _impl->samples_per_second = n;
}

bool BaseEstimator::getRegistration(Eigen::Matrix4f * RAW, Eigen::Matrix4f * UNIT) {

  if (!_impl->successful_registration)
    return false;

  if (RAW)
    *RAW = _impl->registration_raw;

  if (UNIT)
    *UNIT = _impl->registration_unit;

  return true;
}

void BaseEstimator::Impl::update(clock::time_point now) {

  if (!controller) {
    static bool message_shown = false;
    if (!message_shown) {
      GM_ERR("BaseEstimator", "No controller to calibrate");
      message_shown = true;
    }
    return;
  }

  gramods::gmTrack::ButtonsTracker::ButtonsSample buttons;
  if (! controller->getButtons(buttons)) {
    static bool message_shown = false;
    if (!message_shown) {
      GM_ERR("BaseEstimator", "Cannot read controller buttons");
      message_shown = true;
    }
    return;
  }

  if (!collecting) {
    if (buttons.main_button){
      collecting = true;
      GM_INF("BaseEstimator", "going into collect mode");
    } else {
      return;
    }
  }

  if (buttons.main_button) {
    gramods::gmTrack::PoseTracker::PoseSample pose;
    if (! controller->getPose(pose)) {
      static bool message_shown = false;
      if (!message_shown) {
        GM_ERR("BaseEstimator", "Cannot read controller pose");
        message_shown = true;
      }
      return;
    }

    // Zero or less samples per second results in taking only one sample per click
    if (samples_per_second < std::numeric_limits<float>::epsilon()) {
      if (last_sample_time == clock::time_point::min()) {
        GM_INF("BaseEstimator", "collecting a single sample");
        samples.push_back(pose.position);
      }
      last_sample_time = now;
      return;
    }

    // Do not take samples faster than samples_per_second
    if (last_sample_time != clock::time_point::min() &&
        (now - last_sample_time) < std::chrono::milliseconds(int(1000.f/samples_per_second)))
      return;

    GM_VINF("BaseEstimator", "collecting sample");
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

  GM_INF("BaseEstimator", "left collect mode (have " << tracker_positions.size() << " of " << actual_positions.size() << ")");
  GM_VINF("BaseEstimator", "estimated tracked position (" << pos.transpose() << ") out of " << samples.size() << " samples, for actual position (" << actual_positions[tracker_positions.size() - 1].transpose() << ")");

  samples.clear();

  if (tracker_positions.size() < actual_positions.size())
    return;
  GM_INF("BaseEstimator", "have all " << actual_positions.size() << " samples");

  float tracker_data_sph = estimateSphericity(tracker_positions);
  float actual_data_sph = estimateSphericity(actual_positions);

  if ((tracker_data_sph <= planar_sphericity) && (actual_data_sph > planar_sphericity)) {
    GM_WRN("BaseEstimator", "sphericity inconsistency - tracker positions are in a plane but not actual positions");
  } else if ((tracker_data_sph > planar_sphericity) && (actual_data_sph <= planar_sphericity)) {
    GM_WRN("BaseEstimator", "sphericity inconsistency - actual positions are planar but not tracker positions");
  }

  std::vector<Eigen::Vector3f> tracker_data = tracker_positions;
  std::vector<Eigen::Vector3f> actual_data = actual_positions;

  if (std::min(tracker_data_sph, actual_data_sph) <= planar_sphericity) {

    GM_INF("BaseEstimator", "Poor third axis sphericity (" << tracker_data_sph << " and " << actual_data_sph << ") - automatically correcting by expanding samples");

    expandPlanar(actual_data);
    expandPlanar(tracker_data);
  }

  Eigen::Matrix4f M_reg;
  if (!estimateRegistration(tracker_data, actual_data, M_reg))
    return;

  registration_raw = M_reg;
  successful_registration = true;

  GM_VINF("BaseEstimator", "Raw registration matrix:\n" << M_reg);

  Eigen::Matrix4f M_unit;
  estimateUnitRegistration(tracker_data, actual_data, M_reg, M_unit);
  registration_unit = M_unit;

  GM_VINF("BaseEstimator", "Unit registration matrix:\n" << M_unit);
}

void BaseEstimator::Impl::getIQM3D(std::vector<Eigen::Vector3f> samples, Eigen::Vector3f &x) {

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
  for (int idx = samples.size()/4; idx < (3*samples.size())/4; ++idx)
    sum[0] += samples[idx][0];

  std::sort(samples.begin(), samples.end(),
            [](Eigen::Vector3f a, Eigen::Vector3f b){
              return a[1] < b[1];
            });
  for (int idx = samples.size()/4; idx < (3*samples.size())/4; ++idx)
    sum[1] += samples[idx][1];

  std::sort(samples.begin(), samples.end(),
            [](Eigen::Vector3f a, Eigen::Vector3f b){
              return a[2] < b[2];
            });
  for (int idx = samples.size()/4; idx < (3*samples.size())/4; ++idx)
    sum[2] += samples[idx][2];

  x = (1.0 / ((3*samples.size())/4 - samples.size()/4)) * sum;

}


float BaseEstimator::Impl::estimateSphericity(std::vector<Eigen::Vector3f> data) {

  Eigen::Vector3f cp = Eigen::Vector3f::Zero();
  for (auto pt : data)
    cp += pt;
  cp /= data.size();

  Eigen::MatrixXf data_matrix(3, data.size());

  for (size_t idx = 0; idx < data.size(); ++idx) {
    data_matrix.col(idx) = (data[idx] - cp);
  }

  Eigen::JacobiSVD<Eigen::MatrixXf> svd(data_matrix, Eigen::ComputeThinU | Eigen::ComputeThinV);
  auto singular_values = svd.singularValues();
  assert(singular_values.rows() == 3);

  GM_VINF("BaseEstimator", "data matrix:\n" << data_matrix);
  GM_VINF("BaseEstimator", "singular values: " << singular_values.transpose());

  if (singular_values[1] / singular_values[0] < 0.3) //<< Arbitrarily choosen for warning only
    GM_WRN("BaseEstimator", "poor second axis sphericity - points may be too linearly dependent for a good registration estimation");

  return singular_values[2] / singular_values[0];
}


void BaseEstimator::Impl::expandPlanar(std::vector<Eigen::Vector3f> &data) {
  assert(data.size() >= 3);

  Eigen::Vector3f cp = Eigen::Vector3f::Zero();
  for (auto pt : data)
    cp += pt;
  cp /= data.size();

  Eigen::MatrixXf data_matrix(3, data.size());

  for (size_t idx = 0; idx < data.size(); ++idx) {
    data_matrix.col(idx) = (data[idx] - cp);
  }

  Eigen::JacobiSVD<Eigen::MatrixXf> svd(data_matrix, Eigen::ComputeFullU);
  auto U = svd.matrixU();
  auto S = svd.singularValues();

  Eigen::Vector3f data_normal = U.col(2);
  float data_scale = S[0];
  GM_VINF("BaseEstimator", "Estimated data normal: " << data_normal.transpose());
  GM_VINF("BaseEstimator", "Estimated data scale: " << data_scale);

  std::vector<Eigen::Vector3f> new_data;
  new_data.reserve(2 * data.size());

  auto offset = (0.5 * data_scale) * data_normal;
  for (auto pt : data)
    new_data.push_back(pt + offset);
  for (auto pt : data)
    new_data.push_back(pt - offset);

  GM_VVINF("BaseEstimator", "New data:");
  for (auto pt : new_data)
    GM_VVINF("BaseEstimator", "" << pt.transpose());

  data.swap(new_data);
}

bool BaseEstimator::Impl::estimateRegistration(std::vector<Eigen::Vector3f> tracker_data,
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

    GM_VINF("BaseEstimator", "Solving complete system by inverse multiplication");

    // ACTUAL = M_REG  TRACKER
    // M_REG = ACTUAL / TRACKER
    // M_REG = ACTUAL  TRACKER^-1

    Eigen::Matrix4f tracker_set_4f = tracker_set;
    Eigen::Matrix4f tracker_set_inverse = tracker_set_4f.inverse();

    M = actual_set * tracker_set_inverse;

  } else {

    GM_VINF("BaseEstimator", "Solving overdetermined system by inverse multiplication");

    // M_REG  TRACKER = ACTUAL
    // TRACKER^t M_REG^t = ACTUAL^t
    // Ax = b

    auto A = tracker_set.transpose();
    auto b = actual_set.transpose();

    auto qr = A.fullPivHouseholderQr();
    auto x = qr.solve(b);
    M = x.transpose();
  }

  GM_VINF("BaseEstimator", "Raw registration error: " << ((M * tracker_set) - actual_set).norm());

  return true;
}


void BaseEstimator::Impl::estimateUnitRegistration
(std::vector<Eigen::Vector3f> tracker_data,
 std::vector<Eigen::Vector3f> actual_data,
 Eigen::Matrix4f M_raw,
 Eigen::Matrix4f &M_unit) {

  Eigen::JacobiSVD<Eigen::MatrixXf> svd(M_raw.block(0,0,3,3),
                                        Eigen::ComputeFullU | Eigen::ComputeFullV);
  auto U = svd.matrixU();
  auto S = svd.singularValues();
  auto V = svd.matrixV();

  M_unit = Eigen::Matrix4f::Identity();
  M_unit.block(0,0,3,3) = U * V.transpose();

  Eigen::Vector3f tracker_cp = Eigen::Vector3f::Zero();
  for (auto pt : tracker_data)
    tracker_cp += pt;
  tracker_cp /= tracker_data.size();

  Eigen::Vector3f actual_cp = Eigen::Vector3f::Zero();
  for (auto pt : actual_data)
    actual_cp += pt;
  actual_cp /= actual_data.size();

  Eigen::Vector3f offset = (actual_cp - (M_unit * tracker_cp.homogeneous()).hnormalized());
  M_unit.block(0,3,3,1) = offset;
}

END_NAMESPACE_GMTRACK;
