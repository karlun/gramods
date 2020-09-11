
#include <gmGraphics/Viewpoint.hh>

#include <gmCore/Console.hh>

#include <limits>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE(Viewpoint);
GM_OFI_PARAM(Viewpoint, position, Eigen::Vector3f, Viewpoint::setPosition);
GM_OFI_PARAM(Viewpoint, orientation, Eigen::Quaternionf, Viewpoint::setOrientation);
GM_OFI_PARAM(Viewpoint, upDirection, Eigen::Vector3f, Viewpoint::setUpDirection);
GM_OFI_PARAM(Viewpoint, lookAt, Eigen::Vector3f, Viewpoint::setLookAt);
GM_OFI_PARAM(Viewpoint, eyeSeparation, float, Viewpoint::setEyeSeparation);

Eigen::Vector3f Viewpoint::getPosition(Eye eye) {

  eye.validate();

  if (eye.count == 1)
    return position;

  float offset = (eye.idx - 0.5f * (eye.count - 1)) * eye_separation;
  return position + orientation * Eigen::Vector3f(offset, 0.f, 0.f);
}

Eigen::Quaternionf Viewpoint::getOrientation(Eye eye) {
  eye.validate();
  return orientation;
}

void Viewpoint::setPosition(Eigen::Vector3f p) {
  position = p;
}

void Viewpoint::setOrientation(Eigen::Quaternionf q) {
  orientation = q;
}

void Viewpoint::setUpDirection(Eigen::Vector3f up) {
  up_direction = up.normalized();
}

void Viewpoint::setLookAt(Eigen::Vector3f target) {

  static const Eigen::Vector3f z(0, 0, 1);
  static const Eigen::Vector3f y(0, 1, 0);

  auto direction = (position - target).normalized();

  GM_VINF("Viewpoint", "From " << position.transpose()
          << " look at " << target.transpose());

  orientation = Eigen::Quaternionf::FromTwoVectors(z, direction);

  Eigen::Vector3f perp_up = up_direction - direction * up_direction.dot(direction);
  if (perp_up.norm() <= std::numeric_limits<float>::epsilon())
    return;
  perp_up = perp_up.normalized();

  auto Qup = Eigen::Quaternionf::FromTwoVectors(orientation * y, perp_up);
  orientation = Qup * orientation;
}

END_NAMESPACE_GMGRAPHICS;
