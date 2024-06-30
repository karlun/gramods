
#include <gmGraphics/SpatialPlanarView.hh>
#include <gmCore/RunOnce.hh>
#include <gmCore/RunLimited.hh>

#include <limits>
#include <optional>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE_SUB(SpatialPlanarView, MultiscopicView);
GM_OFI_PARAM2(SpatialPlanarView, topLeftCorner, Eigen::Vector3f, setTopLeftCorner);
GM_OFI_PARAM2(SpatialPlanarView, bottomRightCorner, Eigen::Vector3f, setBottomRightCorner);
GM_OFI_PARAM2(SpatialPlanarView, upDirection, Eigen::Vector3f, setUpDirection);
GM_OFI_PARAM2(SpatialPlanarView, position, Eigen::Vector3f, setPosition);
GM_OFI_PARAM2(SpatialPlanarView, orientation, Eigen::Quaternionf, setOrientation);
GM_OFI_PARAM2(SpatialPlanarView, distance, float, setDistance);
GM_OFI_PARAM2(SpatialPlanarView, clipAngles, gmCore::angle4, setClipAngles);

struct SpatialPlanarView::Impl {

  void renderFullPipeline(ViewSettings settings, Eye eye);
  void renderFullPipeline(ViewSettings settings, Eye eye, Viewpoint *viewpoint);

  void calculateCorners();

  std::optional<Eigen::Vector3f> upDirection;
  std::optional<Eigen::Vector3f> topLeftCorner;
  std::optional<Eigen::Vector3f> bottomRightCorner;

  Eigen::Quaternionf orientation = Eigen::Quaternionf::Identity();
  Eigen::Vector3f position = Eigen::Vector3f::Zero();

  std::optional<gmCore::angle4> shape_angles;
  std::optional<float> distance;
};

SpatialPlanarView::SpatialPlanarView()
  : _impl(std::make_unique<Impl>()) {}

SpatialPlanarView::~SpatialPlanarView() {}

void SpatialPlanarView::renderFullPipeline(ViewSettings settings, Eye eye) {
  _impl->renderFullPipeline(settings, eye);
}

void SpatialPlanarView::Impl::renderFullPipeline(ViewSettings settings, Eye eye) {

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  for (auto viewpoint : settings.viewpoints)
    renderFullPipeline(settings, eye, viewpoint.get());
}

void SpatialPlanarView::Impl::renderFullPipeline(ViewSettings settings,
                                                 Eye eye,
                                                 Viewpoint *viewpoint) {

  Eigen::Vector3f x_VP = viewpoint->getPosition(eye);
  Eigen::Quaternionf q_VP = viewpoint->getOrientation(eye);

  if (!topLeftCorner || !bottomRightCorner || !upDirection) {
    GM_RUNONCE(GM_ERR("SpatialPlanarView",
                      "Missing parameters for estimating projection"
                      " - need corners and up, or angles and distance"));
    return;
  }

  auto upDirection = *this->upDirection;
  auto topLeftCorner = *this->topLeftCorner;
  auto bottomRightCorner = *this->bottomRightCorner;

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

  Camera camera(settings);
  camera.setClipPlanes(left, right, bottom, top);
  camera.setPose(x_VP, Q1 * Q0);
  camera.setEye(eye);

  settings.renderNodes(camera);
}

void SpatialPlanarView::Impl::calculateCorners() {

  if (!shape_angles || !distance) return;

  auto distance = *this->distance;
  auto shape_angles = *this->shape_angles;

  topLeftCorner = orientation *
    (distance * Eigen::Vector3f(-std::tan(shape_angles[0]),
                                +std::tan(shape_angles[3]), -1));

  bottomRightCorner = orientation *
    (distance * Eigen::Vector3f(+std::tan(shape_angles[1]),
                                -std::tan(shape_angles[2]), -1));

  upDirection = orientation * Eigen::Vector3f::UnitY();
}


void SpatialPlanarView::setTopLeftCorner(Eigen::Vector3f tlc) {
  _impl->topLeftCorner = tlc;
}

void SpatialPlanarView::setBottomRightCorner(Eigen::Vector3f brc) {
  _impl->bottomRightCorner = brc;
}

void SpatialPlanarView::setUpDirection(Eigen::Vector3f up) {
  _impl->upDirection = up;
  _impl->calculateCorners();
}

void SpatialPlanarView::setPosition(Eigen::Vector3f p) {
  _impl->position = p;
  _impl->calculateCorners();
}

void SpatialPlanarView::setClipAngles(gmCore::angle4 a) {
  _impl->shape_angles = a;
  _impl->calculateCorners();
}

void SpatialPlanarView::setOrientation(Eigen::Quaternionf q) {
  _impl->orientation = q;
  _impl->calculateCorners();
}

void SpatialPlanarView::setDistance(float d) {
  _impl->distance = d;
  _impl->calculateCorners();
}

END_NAMESPACE_GMGRAPHICS;
