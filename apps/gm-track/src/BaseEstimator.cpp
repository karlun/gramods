
#include <BaseEstimator.hh>

#include <thread>
#include <set>

BaseEstimator::BaseEstimator()
  : mode(Mode::UNIT) {
  Eigen::MatrixXf A = Eigen::MatrixXf::Random(3, 3);
  Eigen::VectorXf b = Eigen::VectorXf::Random(3, 3);
  std::cout << "The solution using the QR decomposition is:\n"
            << A.colPivHouseholderQr().solve(b) << std::endl;
}

int BaseEstimator::process() {
  if (!controller) return -1;

  int err;

  std::cout << "Mark origin (hold for multiple samples)" << std::endl;
  std::vector<Eigen::Vector3f> origin_position_samples;
  err = getSamples(origin_position_samples);
  if (err) return err;

  Eigen::Vector3f origin_position;
  err = getIQM3D(origin_position_samples, origin_position);
  if (err) return err;

  if (mode == Mode::UNIT)
    std::cout << "Mark +x direction (hold for multiple samples)" << std::endl;
  else
    std::cout << "Mark +x position (1,0,0) (hold for multiple samples)" << std::endl;
  std::vector<Eigen::Vector3f> x_position_samples;
  err = getSamples(x_position_samples);
  if (err) return err;

  Eigen::Vector3f x_position;
  err = getIQM3D(x_position_samples, x_position);
  if (err) return err;

  if (mode == Mode::UNIT)
    std::cout << "Mark +y direction (hold for multiple samples)" << std::endl;
  else
    std::cout << "Mark +y position (1,0,0) (hold for multiple samples)" << std::endl;
  std::vector<Eigen::Vector3f> y_position_samples;
  err = getSamples(y_position_samples);
  if (err) return err;

  Eigen::Vector3f y_position;
  err = getIQM3D(y_position_samples, y_position);
  if (err) return err;

  if (mode == Mode::UNIT)
    return estimateUnitBase(origin_position, x_position, y_position);

  std::cout << "Mark +z position (1,0,0) (hold for multiple samples)" << std::endl;
  std::vector<Eigen::Vector3f> z_position_samples;
  err = getSamples(z_position_samples);
  if (err) return err;

  Eigen::Vector3f z_position;
  err = getIQM3D(z_position_samples, z_position);
  if (err) return err;

  if (mode == Mode::FREE_SCALE)
    return estimateFreeScaleBase(origin_position, x_position, y_position, z_position);

  if (mode == Mode::UNIFORM_SCALE)
    return estimateUniformScaleBase(origin_position, x_position, y_position, z_position);

  assert(0);
}

int BaseEstimator::getIQM3D(std::vector<Eigen::Vector3f> samples, Eigen::Vector3f &x) {

  if (samples.empty()) return -1;

  if (samples.size() == 1) {
    x = samples.front();
    return 0;
  }

  if (samples.size() < 4) {
    Eigen::Vector3f sum;
    for (auto p : samples) sum += p;
    x = (1.0 / samples.size()) * sum;
    return 0;
  }

  Eigen::Vector3f sum;

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

  return 0;
}

int BaseEstimator::getSamples(std::vector<Eigen::Vector3f> &samples) {

  gramods::gmTrack::ButtonsTracker::ButtonsSample buttons;

  do {
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    if (! controller->getButtons(buttons)) {
      std::cerr << "Could not read buttons" << std::endl;
      return -1;
    }
  } while (! buttons.main_button);

  while (buttons.main_button) {

    gramods::gmTrack::PoseTracker::PoseSample pose;

    if (! controller->getPose(pose)) {
      std::cerr << "Could not read pose" << std::endl;
      return -1;
    }

    samples.push_back(pose.position);

    if (! controller->getButtons(buttons)) {
      std::cerr << "Could not read buttons" << std::endl;
      return -1;
    }
  }

  return 0;
}

int BaseEstimator::estimateUnitBase
(Eigen::Vector3f origin_position,
 Eigen::Vector3f x_position,
 Eigen::Vector3f y_position) {
  auto x = origin_position + (x_position - origin_position).normalized();
  auto y = origin_position + (y_position - origin_position).normalized();
  auto z = x.cross(y);

  Eigen::Vector4f X = x.homogeneous();
  Eigen::Vector4f Y = y.homogeneous();
  Eigen::Vector4f Z = z.homogeneous();
  Eigen::Vector4f W(0, 0, 0, 1);
  
  Eigen::Matrix4f M;
  M <<
    x.homogeneous(),
    y.homogeneous(),
    z.homogeneous(),
    Eigen::Vector4f(0, 0, 0, 1);

  bool invertible;
  float determinant;
  M.computeInverseAndDetWithCheck(base, determinant, invertible);

  if (!invertible) {
    std::cerr << "Invalid positions - could not determine base!" << std::endl;
    return -1;
  }

  return 0;
}

int BaseEstimator::estimateFreeScaleBase
(Eigen::Vector3f origin_position,
 Eigen::Vector3f x_position,
 Eigen::Vector3f y_position,
 Eigen::Vector3f z_position) {}

int BaseEstimator::estimateUniformScaleBase
(Eigen::Vector3f origin_position,
 Eigen::Vector3f x_position,
 Eigen::Vector3f y_position,
 Eigen::Vector3f z_position) {}
