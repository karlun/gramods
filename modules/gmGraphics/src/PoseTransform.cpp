
#include <gmGraphics/PoseTransform.hh>

#include <gmCore/io_eigen.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE_SUB(PoseTransform, Group);
GM_OFI_PARAM2(PoseTransform, position, Eigen::Vector3f, setPosition);
GM_OFI_PARAM2(PoseTransform, orientation, Eigen::Quaternionf, setOrientation);
GM_OFI_PARAM2(PoseTransform, orientationCenter, Eigen::Vector3f, setOrientationCenter);
GM_OFI_PARAM2(PoseTransform, scale, Eigen::Vector3f, setScale);
GM_OFI_PARAM2(PoseTransform, scaleCenter, Eigen::Vector3f, setScaleCenter);
GM_OFI_PARAM2(PoseTransform, scaleOrientation, Eigen::Quaternionf, setScaleOrientation);

struct PoseTransform::Impl {
  Eigen::Affine3f getTransform();

  Eigen::Vector3f position = Eigen::Vector3f::Zero();
  Eigen::Quaternionf orientation = Eigen::Quaternionf::Identity();
  Eigen::Vector3f orientation_center = Eigen::Vector3f::Zero();
  Eigen::Vector3f scale = Eigen::Vector3f(1.f, 1.f, 1.f);
  Eigen::Vector3f scale_center = Eigen::Vector3f::Zero();
  Eigen::Quaternionf scale_orientation = Eigen::Quaternionf::Identity();
  std::optional<Eigen::Affine3f> cache_transform;
};

PoseTransform::PoseTransform() : _impl(std::make_unique<Impl>()) {}
PoseTransform::~PoseTransform() {}

void PoseTransform::setPosition(Eigen::Vector3f p) {
  _impl->position = p;
  _impl->cache_transform = std::nullopt;
}

Eigen::Vector3f PoseTransform::getPosition() {
  return _impl->position;
}

void PoseTransform::setOrientation(Eigen::Quaternionf q) {
  _impl->orientation = q;
  _impl->cache_transform = std::nullopt;
}

Eigen::Quaternionf PoseTransform::getOrientation() {
  return _impl->orientation;
}

void PoseTransform::setOrientationCenter(Eigen::Vector3f p) {
  _impl->orientation_center = p;
  _impl->cache_transform = std::nullopt;
}

Eigen::Vector3f PoseTransform::getOrientationCenter() {
  return _impl->orientation_center;
}

void PoseTransform::setScale(Eigen::Vector3f p) {
  _impl->scale = p;
  _impl->cache_transform = std::nullopt;
}

Eigen::Vector3f PoseTransform::getScale() {
  return _impl->scale;
}

void PoseTransform::setScaleCenter(Eigen::Vector3f p) {
  _impl->scale_center = p;
  _impl->cache_transform = std::nullopt;
}

Eigen::Vector3f PoseTransform::getScaleCenter() {
  return _impl->scale_center;
}

void PoseTransform::setScaleOrientation(Eigen::Quaternionf q) {
  _impl->scale_orientation = q;
  _impl->cache_transform = std::nullopt;
}

Eigen::Quaternionf PoseTransform::getScaleOrientation() {
  return _impl->scale_orientation;
}

Eigen::Affine3f PoseTransform::getTransform() {
  return _impl->getTransform();
}

Eigen::Affine3f PoseTransform::Impl::getTransform() {
  if (cache_transform) return *cache_transform;

  cache_transform = Eigen::Translation3f(position) *            //
                    Eigen::Translation3f(orientation_center) *  //
                    orientation *                               //
                    Eigen::Translation3f(-orientation_center) * //
                    Eigen::Translation3f(scale_center) *        //
                    scale_orientation *                         //
                    Eigen::AlignedScaling3f(scale) *            //
                    scale_orientation.conjugate() *             //
                    Eigen::Translation3f(-scale_center);
  return *cache_transform;
}

END_NAMESPACE_GMGRAPHICS;
