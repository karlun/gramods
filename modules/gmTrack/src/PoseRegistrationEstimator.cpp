
#include <gmTrack/PoseRegistrationEstimator.hh>
#include <gmTrack/SampleCollector.impl.hh>

#include <gmTrack/ButtonsMapper.hh>

#include <gmCore/RunOnce.hh>
#include <gmCore/Console.hh>

#include <Eigen/LU>

#include <limits>
#include <type_traits>

BEGIN_NAMESPACE_GMTRACK;

GM_OFI_DEFINE_SUB(PoseRegistrationEstimator, SampleCollector);
GM_OFI_PARAM2(PoseRegistrationEstimator, actualPosition, Eigen::Vector3f, addActualPosition);

struct PoseRegistrationEstimator::Impl : SampleCollector::Impl {

  typedef gmCore::Updateable::clock clock;

  std::vector<Eigen::Vector3f> actual_positions;

  void update(clock::time_point t) override;

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

  bool estimateRegistration(const std::vector<Eigen::Vector3f> &tracker_data,
                            const std::vector<Eigen::Vector3f> &actual_data,
                            Eigen::Matrix4f &M);

  void estimateUnitRegistration(const std::vector<Eigen::Vector3f> &tracker_data,
                                const std::vector<Eigen::Vector3f> &actual_data,
                                const Eigen::Matrix4f &M_raw,
                                Eigen::Matrix4f &M_unit);

  void checkResult(const std::vector<Eigen::Vector3f> &tracker_data,
                   const std::vector<Eigen::Vector3f> &actual_data,
                   const Eigen::Matrix4f &M_unit,
                   std::string type);

  float planar_sphericity = 0.3f;

  Eigen::Matrix4f registration_raw;
  Eigen::Matrix4f registration_unit;
  bool successful_registration = false;

  size_t position_to_collect = std::numeric_limits<size_t>::max();
};

PoseRegistrationEstimator::PoseRegistrationEstimator()
  : SampleCollector(new Impl) {}

PoseRegistrationEstimator::~PoseRegistrationEstimator() {}

void PoseRegistrationEstimator::addActualPosition(Eigen::Vector3f p) {
  auto _impl = static_cast<PoseRegistrationEstimator::Impl *>(this->_impl.get());
  _impl->actual_positions.push_back(p);
}

bool PoseRegistrationEstimator::getRegistration(Eigen::Matrix4f * RAW, Eigen::Matrix4f * UNIT) {
  auto _impl = static_cast<PoseRegistrationEstimator::Impl *>(this->_impl.get());

  if (!_impl->successful_registration)
    return false;

  if (RAW)
    *RAW = _impl->registration_raw;

  if (UNIT)
    *UNIT = _impl->registration_unit;

  return true;
}

void PoseRegistrationEstimator::Impl::update(clock::time_point now) {

  if (actual_positions.empty())
    return;

  SampleCollector::Impl::update(now);

  if (tracker_positions.size() < actual_positions.size()) {
    if (tracker_positions.size() != position_to_collect) {
      position_to_collect = tracker_positions.size();
      GM_INF("PoseRegistrationEstimator",
             "Point to collect: "
                 << actual_positions[position_to_collect].transpose());
    }
    return;
  }

  GM_INF("PoseRegistrationEstimator", "have all " << actual_positions.size() << " samples");
  performRegistration();
}

void PoseRegistrationEstimator::performRegistration() {
  auto _impl = static_cast<PoseRegistrationEstimator::Impl *>(this->_impl.get());
  _impl->performRegistration();
}

void PoseRegistrationEstimator::Impl::performRegistration() {

  if (actual_positions.size() < 3 ||
      tracker_positions.size() != actual_positions.size()) {
    GM_ERR("PoseRegistrationEstimator",
           "Registration triggered with incorrect number of samples available: "
               << actual_positions.size() << " and " << tracker_positions.size()
               << " actual and tracker positions, respectively");
    return;
  }

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

    GM_WRN("PoseRegistrationEstimator",
           "Samples will be manipulated to compensate for poor sphericity ("
               << tracker_data_sph << " and " << actual_data_sph
               << " for tracker data and actual data, respectively)");

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
  checkResult(tracker_data, actual_data, M_reg, "perfect");

  Eigen::Matrix4f M_unit;
  estimateUnitRegistration(tracker_data, actual_data, M_reg, M_unit);
  registration_unit = M_unit;

  GM_DBG1("PoseRegistrationEstimator", "Unit registration matrix:\n" << M_unit);
  checkResult(tracker_data, actual_data, M_unit, "unit");

  tracker_positions.clear();
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
        idx0 = (int)idx;
      }
    }

    float best_value1 = 0.f;
    for (size_t idx = 0; idx < data.size(); ++idx) {
      if (idx == (size_t)idx0) continue;
      float value = fabsf(data_Y.dot((data[idx] - cp)));
      if (value > best_value1) {
        best_value1 = value;
        idx1 = (int)idx;
      }
    }

    GM_DBG1("PoseRegistrationEstimator",
            "Estimated primary samples: " << idx0 << " (" << best_value0
                                          << ") and " << idx1 << "("
                                          << best_value1 << ")");
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
(const std::vector<Eigen::Vector3f> &tracker_data,
 const std::vector<Eigen::Vector3f> &actual_data,
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
(const std::vector<Eigen::Vector3f> &tracker_data,
 const std::vector<Eigen::Vector3f> &actual_data,
 const Eigen::Matrix4f &M_raw,
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

void PoseRegistrationEstimator::Impl::checkResult(
    const std::vector<Eigen::Vector3f> &tracker_data,
    const std::vector<Eigen::Vector3f> &actual_data,
    const Eigen::Matrix4f &M_unit,
    std::string type) {

  float worst_sqr_offset = 0.f;
  for (size_t idx = 0; idx < std::min(tracker_data.size(), actual_data.size());
       ++idx) {
    Eigen::Vector3f proj =
        (M_unit * tracker_data[idx].homogeneous()).hnormalized();
    Eigen::Vector3f offset = actual_data[idx] - proj;
    float sqr_offset = offset.squaredNorm();
    worst_sqr_offset = std::max(worst_sqr_offset, sqr_offset);
  }

  float worst_offset = std::sqrt(worst_sqr_offset);
  if (worst_offset > warning_threshold) {
    GM_WRN("PoseRegistrationEstimator",
           "Worst " << type << " reprojected tracker-point has an offset of "
                    << worst_offset << " m!");
  } else {
    GM_DBG1("PoseRegistrationEstimator",
            "Worst " << type << " reprojected tracker-point has an offset of "
                     << worst_offset << " m.");
  }
}

END_NAMESPACE_GMTRACK;
