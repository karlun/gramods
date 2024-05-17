
#include <gmGraphics/Camera.hh>

#include <gmCore/Console.hh>
#include <gmCore/RunLimited.hh>
#include <gmCore/PreConditionViolation.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

Eigen::Matrix4f Camera::getProjectionMatrix() const {
  if (!near || !far)
    throw gmCore::PreConditionViolation(
        "near and far plane distances required");

  GM_DBG2("Camera", "Creating projection matrix from ("
          << left << ", " << right << ", "
          << bottom << ", " << top << ", "
          << *near << ", " << *far << ")");

  Eigen::Matrix4f Mp = Eigen::Matrix4f::Zero();

  Mp(0,0) = 2.f / (right - left);
  Mp(1,1) = 2.f / (top - bottom);
  Mp(0,2) = (right + left) / (right - left);
  Mp(1,2) = (top + bottom) / (top - bottom);
  Mp(2,2) = -(*far + *near) / (*far - *near);
  Mp(3,2) = -1.f;
  Mp(2,3) = -(2.f * *far * *near) / (*far - *near);

  return Mp;
}

Eigen::Affine3f Camera::getViewMatrix() const {

  {
    Eigen::AngleAxis<float> aa(orientation);
    GM_DBG2("Camera", "Creating view matrix from ("
            << position.transpose() << ", "
            << aa.axis().transpose() << " " << aa.angle() << ")");
  }

  Eigen::Affine3f Mv = Eigen::Affine3f::Identity();

  auto q = orientation.conjugate();
  Mv.linear() = q.toRotationMatrix();
  Mv.translation() = - (Mv.linear() * position);

  return Mv;
}

void Camera::setFieldOfView(float fov_h, float fov_v) {
  right = tanf(0.5f * fov_h);
  left = -right;
  top = tanf(0.5f * fov_v);
  bottom = -top;
}

void Camera::setClipAngles(float l, float r, float b, float t) {
  left = -tanf(l);
  right = tanf(r);
  bottom = -tanf(b);
  top = tanf(t);
}

bool Camera::setLookAtPoints(Eigen::Vector3f eye_position,
                             const std::vector<Eigen::Vector3f> &pts,
                             bool symmetric,
                             Eigen::Vector3f up) {

  if (pts.size() <= 3) {
    GM_DBG2("Camera", "Too few points to look at (" << pts.size() << ").");
    return false;
  }

  Eigen::Vector3f center = Eigen::Vector3f::Zero();
  for (auto pt : pts)
    center += pt;
  center /= (float)pts.size();

  Eigen::Vector3f direction = center - eye_position;
  float direction_norm = direction.norm();
  if (direction_norm < std::numeric_limits<float>::epsilon()) {
    GM_RUNLIMITED(
        GM_ERR("Camera", "Cannot look at points surrounding viewer position."),
        1);
    return false;
  }
  direction /= direction_norm;

  for (auto pt : pts)
    if ((pt - eye_position).dot(direction) < std::numeric_limits<float>::epsilon()) {
      GM_RUNLIMITED(
          GM_ERR("Camera",
                 "Cannot look at points surrounding viewer position."),
          1);
      return false;
    }

  Eigen::Vector3f data_X, data_Y;

  if (up.squaredNorm() > 0.5) {

    data_X = direction.cross(up);
    if (data_X.norm() < std::numeric_limits<float>::epsilon())
      data_X = direction.cross(
          Eigen::Vector3f(direction[1], direction[2], direction[0]));
    data_X = data_X.normalized();
    data_Y = data_X.cross(direction).normalized();

  } else {

    Eigen::MatrixXf data_matrix(3, pts.size());

    for (size_t idx = 0; idx < pts.size(); ++idx)
      data_matrix.col(idx) = pts[idx] - center;

    Eigen::JacobiSVD<Eigen::MatrixXf> svd(data_matrix, Eigen::ComputeFullU);
    auto U = svd.matrixU();

    // Find camera coordinates
    data_X = U.col(0);
    float dim_power_X = direction.cross(data_X).norm();
    for (size_t idx = 1; idx < 3; ++idx) {
      Eigen::Vector3f data_cand = U.col(idx);
      float dim_power_cand = direction.cross(data_cand).norm();
      if (dim_power_cand > dim_power_X) {
        data_X = data_cand;
        dim_power_X = dim_power_cand;
      }
    }
    data_Y = data_X.cross(direction).normalized();
    data_X = direction.cross(data_Y).normalized();
  }

  // Find camera orientation
  Eigen::Quaternionf R0 =
      Eigen::Quaternionf::FromTwoVectors(-Eigen::Vector3f::UnitZ(), direction);
  Eigen::Vector3f data_Y0 = R0.conjugate() * data_Y;
  Eigen::Quaternionf R1 = Eigen::Quaternionf::FromTwoVectors(Eigen::Vector3f::UnitY(), data_Y0);
  orientation = R0 * R1;

  left = std::numeric_limits<float>::max();
  right = std::numeric_limits<float>::lowest();
  bottom = std::numeric_limits<float>::max();
  top = std::numeric_limits<float>::lowest();

  // Find camera frustum planes
  for (auto pt : pts) {

    Eigen::Vector3f pos = pt - eye_position;

    float D = 1.f / pos.dot(direction);
    float R = pos.dot(data_X) * D;
    float U = pos.dot(data_Y) * D;

    left = std::min(left, R);
    right = std::max(right, R);
    bottom = std::min(bottom, U);
    top = std::max(top, U);
  }

  position = eye_position;

  if (symmetric) {
    left = std::min(left, -right);
    right = std::max(right, -left);
    bottom = std::min(bottom, -top);
    top = std::max(top, -bottom);
  }

  GM_DBG1("Camera",
          "Set to look at "
              << pts.size() << " point from " << position.transpose()
              << ": direction = " << direction.transpose() << ", (l,r,b,t) = ("
              << left << "," << right << "," << bottom << "," << top << ")");

  return true;
}

END_NAMESPACE_GMGRAPHICS;
