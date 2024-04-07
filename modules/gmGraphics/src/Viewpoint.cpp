
#include <gmGraphics/Viewpoint.hh>

#include <gmCore/Console.hh>

#include <limits>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE(Viewpoint);
GM_OFI_PARAM2(Viewpoint, position, Eigen::Vector3f, setPosition);
GM_OFI_PARAM2(Viewpoint, orientation, Eigen::Quaternionf, setOrientation);
GM_OFI_PARAM2(Viewpoint, upDirection, Eigen::Vector3f, setUpDirection);
GM_OFI_PARAM2(Viewpoint, lookAt, Eigen::Vector3f, setLookAt);
GM_OFI_PARAM2(Viewpoint, eyeSeparation, float, setEyeSeparation);

Eigen::Vector3f Viewpoint::getPosition(Eye eye) {

  eye.validate();

  if (eye.count == 1)
    return position;

  float offset = (eye.idx - 0.5f * (eye.count - 1)) * eye_separation;
  return position + orientation * Eigen::Vector3f(offset, 0.f, 0.f);
}

namespace {
Eigen::Quaternionf getLookAtOrientation(Eigen::Vector3f position,
                                        Eigen::Vector3f target,
                                        Eigen::Vector3f up_direction) {

  static const Eigen::Vector3f z(0, 0, 1);
  static const Eigen::Vector3f y(0, 1, 0);

  auto direction = (position - target).normalized();

  GM_DBG2("Viewpoint", "From " << position.transpose()
          << " look at " << target.transpose());

  Eigen::Quaternionf R0 = Eigen::Quaternionf::FromTwoVectors(z, direction);

  Eigen::Vector3f perp_up = up_direction - direction * up_direction.dot(direction);
  if (perp_up.norm() <= std::numeric_limits<float>::epsilon())
    return R0;
  perp_up = perp_up.normalized();

  auto Rup = Eigen::Quaternionf::FromTwoVectors(R0 * y, perp_up);
  return Rup * R0;
}
}

Eigen::Quaternionf Viewpoint::getOrientation(Eye eye) {
  eye.validate();

  if (look_at) return getLookAtOrientation(position, *look_at, up_direction);
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

END_NAMESPACE_GMGRAPHICS;
