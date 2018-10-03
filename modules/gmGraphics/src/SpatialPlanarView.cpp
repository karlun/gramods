
#include <gmGraphics/SpatialPlanarView.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE_SUB(SpatialPlanarView, View);
GM_OFI_PARAM(SpatialPlanarView, topLeftCorner, gmTypes::float3, SpatialPlanarView::setTopLeftCorner);
GM_OFI_PARAM(SpatialPlanarView, bottomRightCorner, gmTypes::float3, SpatialPlanarView::setBottomRightCorner);
GM_OFI_PARAM(SpatialPlanarView, upDirection, gmTypes::float3, SpatialPlanarView::setUpDirection);

void SpatialPlanarView::renderFullPipeline(ViewSettings settings) {

  if (viewpoint)
    settings.viewpoint = viewpoint;

  Eigen::Vector3f x_VP;
  Eigen::Quaternionf q_VP;

  if (settings.viewpoint) {
    x_VP = settings.viewpoint->getPosition();
    q_VP = settings.viewpoint->getOrientation();
  } else {
    static bool message_shown = false;
    if (!message_shown)
      GM_WRN("SpatialPlanarView", "No viewpoint available - using zero position and rotation");
    message_shown = true;
  }

  switch (settings.eye_to_render) {
  case Viewpoint::Eye::LEFT:
    x_VP -= q_VP * Eigen::Vector3f(0.5f * settings.eye_separation, 0.f, 0.f);
    break;
  case Viewpoint::Eye::RIGHT:
    x_VP += q_VP * Eigen::Vector3f(0.5f * settings.eye_separation, 0.f, 0.f);
  }

  auto up = upDirection.normalized();

  // Display center and normal
  auto center = 0.5f * (bottomRightCorner + topLeftCorner);
  auto display_normal = (bottomRightCorner - topLeftCorner).cross(up);
  display_normal = display_normal.normalized();
  auto rightDirection = up.cross(display_normal);
  rightDirection = rightDirection.normalized();

  auto distance = (x_VP - center).dot(display_normal);

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
  camera.setPlanes(left, right, top, bottom);
  camera.setPose(x_VP, Q1 * Q0);

  for (auto renderer : settings.renderers)
    renderer->render(camera);
}

END_NAMESPACE_GMGRAPHICS;
