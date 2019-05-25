
#include <gmGraphics/Camera.hh>

#include <gmCore/Console.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

Eigen::Matrix4f Camera::getProjectionMatrix(float near, float far) {

  GM_VINF("Camera", "Creating projection matrix from ("
          << left << ", " << right << ", "
          << bottom << ", " << top << ", "
          << near << ", " << far << ")");

  Eigen::Matrix4f Mp = Eigen::Matrix4f::Zero();

  Mp(0,0) = 2.f / (right - left);
  Mp(1,1) = 2.f / (top - bottom);
  Mp(0,2) = (right + left) / (right - left);
  Mp(1,2) = (top + bottom) / (top - bottom);
  Mp(2,2) = -(far + near) / (far - near);
  Mp(3,2) = -1.f;
  Mp(2,3) = -(2.f * far * near) / (far - near);

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

Eigen::Vector3f Camera::getPosition() {
  return position;
}

Eigen::Quaternionf Camera::getOrientation() {
  return orientation;
}

void Camera::setFieldOfView(float fov_h, float fov_v) {
  right = tanf(0.5f * fov_h);
  left = -right;
  top = tanf(0.5f * fov_v);
  bottom = -top;
}

void Camera::setAngles(float l, float r, float b, float t) {
  left = -atanf(l);
  right = atanf(r);
  bottom = -atanf(b);
  top = atanf(t);
}

END_NAMESPACE_GMGRAPHICS;
