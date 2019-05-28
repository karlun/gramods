
#include <gmGraphics/SpatialPlanarView.hh>
#include <gmCore/RunOnce.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE_SUB(SpatialPlanarView, StereoscopicView);
GM_OFI_PARAM(SpatialPlanarView, topLeftCorner, gmTypes::float3, SpatialPlanarView::setTopLeftCorner);
GM_OFI_PARAM(SpatialPlanarView, bottomRightCorner, gmTypes::float3, SpatialPlanarView::setBottomRightCorner);
GM_OFI_PARAM(SpatialPlanarView, upDirection, gmTypes::float3, SpatialPlanarView::setUpDirection);

void SpatialPlanarView::renderFullPipeline(ViewSettings settings, Eye eye) {

  Eigen::Vector3f x_VP = Eigen::Vector3f::Zero();
  Eigen::Quaternionf q_VP = Eigen::Quaternionf::Identity();

  if (settings.viewpoint) {
    x_VP = settings.viewpoint->getPosition();
    q_VP = settings.viewpoint->getOrientation();
  } else {
    GM_RUNONCE(GM_WRN("SpatialPlanarView", "No viewpoint available - using zero position and rotation"));
  }

  switch (eye) {
  case Eye::LEFT:
    x_VP -= q_VP * Eigen::Vector3f(0.5f * eye_separation, 0.f, 0.f);
    break;
  case Eye::RIGHT:
    x_VP += q_VP * Eigen::Vector3f(0.5f * eye_separation, 0.f, 0.f);
  }

  auto up = upDirection.normalized();

  auto display_normal = (bottomRightCorner - topLeftCorner).cross(up);
  display_normal = display_normal.normalized();
  auto rightDirection = up.cross(display_normal);
  rightDirection = rightDirection.normalized();

  auto distance = (x_VP - topLeftCorner).dot(display_normal);
  auto center = x_VP - distance * display_normal;

  // Map geometry onto near plane
  float ratio = 1.f / distance;
  float left = ratio * (topLeftCorner - center).dot(rightDirection);
  float right = ratio * (bottomRightCorner - center).dot(rightDirection);
  float top = ratio * (topLeftCorner - center).dot(up);
  float bottom = ratio * (bottomRightCorner - center).dot(up);

  static const Eigen::Vector3f z(0, 0, 1);
  static const Eigen::Vector3f y(0, 1, 0);
  auto Q0 = Eigen::Quaternionf::FromTwoVectors(z, display_normal);
  auto Q1 = Eigen::Quaternionf::FromTwoVectors(Q0 * y, up);

  Camera camera;
  camera.setClipPlanes(left, right, bottom, top);
  camera.setPose(x_VP, Q1 * Q0);

  for (auto renderer : settings.renderers)
    renderer->render(camera);
}

END_NAMESPACE_GMGRAPHICS;
