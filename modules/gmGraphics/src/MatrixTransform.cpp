
#include <gmGraphics/MatrixTransform.hh>

#include <gmCore/io_eigen.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE_SUB(MatrixTransform, Group);
GM_OFI_PARAM2(MatrixTransform, matrix, Eigen::Matrix4f, setMatrix);

struct MatrixTransform::Impl {
  Eigen::Affine3f getTransform();

  Eigen::Matrix4f matrix = Eigen::Matrix4f::Identity();
  std::optional<Eigen::Affine3f> cache_transform;
};

MatrixTransform::MatrixTransform() : _impl(std::make_unique<Impl>()) {}
MatrixTransform::~MatrixTransform() {}

void MatrixTransform::setMatrix(Eigen::Matrix4f m) {
  _impl->matrix = m;
  _impl->cache_transform = std::nullopt;
}

Eigen::Matrix4f MatrixTransform::getMatrix() const {
  return _impl->matrix;
}

Eigen::Affine3f MatrixTransform::getTransform() {
  return _impl->getTransform();
}

Eigen::Affine3f MatrixTransform::Impl::getTransform() {
  if (cache_transform) return *cache_transform;

  cache_transform = matrix;
  return *cache_transform;
}

END_NAMESPACE_GMGRAPHICS;
