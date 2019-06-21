
#include <gmGraphics/Viewpoint.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE(Viewpoint);
GM_OFI_PARAM(Viewpoint, position, Eigen::Vector3f, Viewpoint::setPosition);
GM_OFI_PARAM(Viewpoint, orientation, Eigen::Quaternionf, Viewpoint::setOrientation);
GM_OFI_PARAM(Viewpoint, upDirection, Eigen::Vector3f, Viewpoint::setUpDirection);
GM_OFI_PARAM(Viewpoint, lookAt, Eigen::Vector3f, Viewpoint::setLookAt);

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
