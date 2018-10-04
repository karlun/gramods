
#include <gmGraphics/Viewpoint.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE(Viewpoint);
GM_OFI_PARAM(Viewpoint, position, gmTypes::float3, Viewpoint::setPosition);
GM_OFI_PARAM(Viewpoint, quaternion, gmTypes::float4, Viewpoint::setQuaternion);
GM_OFI_PARAM(Viewpoint, axisAngle, gmTypes::float4, Viewpoint::setAxisAngle);
GM_OFI_PARAM(Viewpoint, upDirection, gmTypes::float3, Viewpoint::setUpDirection);
GM_OFI_PARAM(Viewpoint, lookAt, gmTypes::float3, Viewpoint::setLookAt);

void Viewpoint::setLookAt(gmTypes::float3 pt) {

  static const Eigen::Vector3f z(0, 0, 1);
  static const Eigen::Vector3f y(0, 1, 0);

  Eigen::Vector3f target(pt[0], pt[1], pt[2]);
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
