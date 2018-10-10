
#include <gmGraphics/Camera.hh>

#include <gmCore/Console.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

Eigen::Matrix4f Camera::getProjectionMatrix(float near, float far) {

  GM_VINF("Camera", "Creating projection matrix from ("
          << left << ", " << right << ", "
          << bottom << ", " << top << ", "
          << near << ", " << far << ")");

  Eigen::Matrix4f Mp = Eigen::Matrix4f::Zero();

  Mp(0,0) = 2.0 / (right - left);
  Mp(1,1) = 2.0 / (top - bottom);
  Mp(0,2) = 2.0 * (right + left) / (right - left);
  Mp(1,2) = 2.0 * (top + bottom) / (top - bottom);
  Mp(2,2) = -(far + near) / (far - near);
  Mp(3,2) = -1.0;
  Mp(2,3) = -(2.0 * far * near) / (far - near);

  return Mp;
}

Eigen::Affine3f Camera::getViewMatrix() {

  {
    Eigen::AngleAxis<float> aa(orientation);
    GM_VINF("Camera", "Creating view matrix from ("
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
  right = atan(0.5 * fov_h);
  left = -right;
  top = atan(0.5 * fov_v);
  bottom = -top;
}

END_NAMESPACE_GMGRAPHICS;
