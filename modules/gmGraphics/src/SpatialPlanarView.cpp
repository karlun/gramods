
#include <gmGraphics/SpatialPlanarView.hh>
#include <gmCore/RunOnce.hh>
#include <gmCore/RunLimited.hh>

#include <limits>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE_SUB(SpatialPlanarView, MultiscopicView);
GM_OFI_PARAM(SpatialPlanarView, topLeftCorner, Eigen::Vector3f, SpatialPlanarView::setTopLeftCorner);
GM_OFI_PARAM(SpatialPlanarView, bottomRightCorner, Eigen::Vector3f, SpatialPlanarView::setBottomRightCorner);
GM_OFI_PARAM(SpatialPlanarView, upDirection, Eigen::Vector3f, SpatialPlanarView::setUpDirection);
GM_OFI_PARAM(SpatialPlanarView, position, Eigen::Vector3f, SpatialPlanarView::setPosition);
GM_OFI_PARAM(SpatialPlanarView, orientation, Eigen::Quaternionf, SpatialPlanarView::setOrientation);
GM_OFI_PARAM(SpatialPlanarView, distance, float, SpatialPlanarView::setDistance);
GM_OFI_PARAM(SpatialPlanarView, clipAngles, gmTypes::angle4, SpatialPlanarView::setClipAngles);

struct SpatialPlanarView::Impl {

  void renderFullPipeline(ViewSettings settings, Eye eye);

  void calculateCorners();

  Eigen::Vector3f upDirection;
  Eigen::Vector3f topLeftCorner;
  Eigen::Vector3f bottomRightCorner;

  bool have_corner_tl = false;
  bool have_corner_br = false;
  bool have_up = false;

  Eigen::Quaternionf orientation = Eigen::Quaternionf::Identity();
  Eigen::Vector3f position = Eigen::Vector3f::Zero();

  gmTypes::angle4 shape_angles;
  double distance;

  bool have_shape_angles = false;
  bool have_distance = false;
};

SpatialPlanarView::SpatialPlanarView()
  : _impl(std::make_unique<Impl>()) {}

SpatialPlanarView::~SpatialPlanarView() {}

void SpatialPlanarView::renderFullPipeline(ViewSettings settings, Eye eye) {
  _impl->renderFullPipeline(settings, eye);
}

void SpatialPlanarView::Impl::renderFullPipeline(ViewSettings settings, Eye eye) {

  Eigen::Vector3f x_VP = Eigen::Vector3f::Zero();
  Eigen::Quaternionf q_VP = Eigen::Quaternionf::Identity();

  if (settings.viewpoint) {
    x_VP = settings.viewpoint->getPosition(eye);
    q_VP = settings.viewpoint->getOrientation(eye);
  } else {
    GM_RUNONCE(GM_WRN("SpatialPlanarView", "No viewpoint available - using zero position and rotation"));
  }

  if (!have_corner_tl || !have_corner_br || !have_up) {
    GM_RUNLIMITED(GM_ERR("SpatialPlanarView", "Missing parameters for estimating projection."), 1);
    return;
  }

  auto up = upDirection.normalized();

  auto display_normal = (bottomRightCorner - topLeftCorner).cross(up);
  display_normal = display_normal.normalized();
  auto rightDirection = up.cross(display_normal);
  rightDirection = rightDirection.normalized();

  auto distance = (x_VP - topLeftCorner).dot(display_normal);
  auto center = x_VP - distance * display_normal;

  if (distance < std::numeric_limits<float>::epsilon()) {
    GM_RUNLIMITED(GM_ERR("SpatialPlanarView", "Cannot render view with viewpoint on the plane"), 1);
    return;
  }

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
  camera.setEye(eye);

  float near, far;
  Renderer::getNearFar(settings.renderers, camera, near, far);

  for (auto renderer : settings.renderers)
    renderer->render(camera, near, far);
}

void SpatialPlanarView::Impl::calculateCorners() {

  if (!have_shape_angles || !have_distance)
    return;

  topLeftCorner = orientation *
    (distance * Eigen::Vector3f(-std::tan(shape_angles[0]),
                                +std::tan(shape_angles[3]), -1));

  bottomRightCorner = orientation *
    (distance * Eigen::Vector3f(+std::tan(shape_angles[1]),
                                -std::tan(shape_angles[2]), -1));

  upDirection = orientation * Eigen::Vector3f::UnitY();

  have_corner_tl = true;
  have_corner_br = true;
  have_up = true;
}


void SpatialPlanarView::setTopLeftCorner(Eigen::Vector3f tlc) {
  _impl->topLeftCorner = tlc;
  _impl->have_corner_tl = true;
}

void SpatialPlanarView::setBottomRightCorner(Eigen::Vector3f brc) {
  _impl->bottomRightCorner = brc;
  _impl->have_corner_br = true;
}

void SpatialPlanarView::setUpDirection(Eigen::Vector3f up) {
  _impl->upDirection = up;
  _impl->have_up = true;
  _impl->calculateCorners();
}

void SpatialPlanarView::setPosition(Eigen::Vector3f p) {
  _impl->position = p;
  _impl->calculateCorners();
}

void SpatialPlanarView::setClipAngles(gmTypes::angle4 a) {
  _impl->shape_angles = a;
  _impl->have_shape_angles = true;
  _impl->calculateCorners();
}

void SpatialPlanarView::setOrientation(Eigen::Quaternionf q) {
  _impl->orientation = q;
  _impl->calculateCorners();
}

void SpatialPlanarView::setDistance(float d) {
  _impl->distance = d;
  _impl->have_distance = true;
  _impl->calculateCorners();
}

END_NAMESPACE_GMGRAPHICS;
