
#include <gmGraphics/GeometryCorrectedProjectorView.hh>

#include <gmGraphics/GLUtils.hh>
#include <gmCore/RunOnce.hh>

#include <gmGraphics/OffscreenRenderTarget.hh>
#include <gmGraphics/RasterProcessor.hh>

#include <Eigen/Eigen>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE_SUB(GeometryCorrectedProjectorView, StereoscopicView);
GM_OFI_PARAM(GeometryCorrectedProjectorView, bufferWidth, int, GeometryCorrectedProjectorView::setBufferWidth);
GM_OFI_PARAM(GeometryCorrectedProjectorView, bufferHeight, int, GeometryCorrectedProjectorView::setBufferHeight);
GM_OFI_PARAM(GeometryCorrectedProjectorView, topLeftCorner, gmTypes::float3, GeometryCorrectedProjectorView::setTopLeftCorner);
GM_OFI_PARAM(GeometryCorrectedProjectorView, bottomRightCorner, gmTypes::float3, GeometryCorrectedProjectorView::setBottomRightCorner);
GM_OFI_PARAM(GeometryCorrectedProjectorView, position, gmTypes::float3, GeometryCorrectedProjectorView::setPosition);
GM_OFI_PARAM(GeometryCorrectedProjectorView, extrinsics, gmTypes::float12, GeometryCorrectedProjectorView::setExtrinsics);
GM_OFI_PARAM(GeometryCorrectedProjectorView, intrinsics, gmTypes::float4, GeometryCorrectedProjectorView::setIntrinsics);
GM_OFI_PARAM(GeometryCorrectedProjectorView, angles, gmTypes::float4, GeometryCorrectedProjectorView::setAngles);
GM_OFI_PARAM(GeometryCorrectedProjectorView, quaternion, gmTypes::float4, GeometryCorrectedProjectorView::setQuaternion);
GM_OFI_PARAM(GeometryCorrectedProjectorView, axisAngle, gmTypes::float4, GeometryCorrectedProjectorView::setAxisAngle);
GM_OFI_PARAM(GeometryCorrectedProjectorView, eulerAngles, gmTypes::float3, GeometryCorrectedProjectorView::setEulerAngles);
GM_OFI_POINTER(GeometryCorrectedProjectorView, geometry, Geometry, GeometryCorrectedProjectorView::setGeometry);

struct GeometryCorrectedProjectorView::Impl {

  static const std::string fragment_code;
  static const std::string mapper_pattern;

  OffscreenRenderTarget render_target;
  RasterProcessor raster_processor;

  bool is_setup = false;
  bool is_functional = false;

  void renderFullPipeline(ViewSettings settings, Eye eye);

  bool setCamera(Camera &c);
  bool setCameraShapeFromIntrinsics(Camera &c);
  bool setCameraShapeFromAngles(Camera &c);
  bool setCameraShapeFromCorners(Camera &c);

  std::string createFragmentCode();

  size_t buffer_width = 2048;
  size_t buffer_height = 2048;

  Eigen::Vector3f shape_corner_tl;
  Eigen::Vector3f shape_corner_br;
  gmTypes::float4 shape_intrinsics;
  gmTypes::float4 shape_angles;

  bool have_shape_corner_tl = false;
  bool have_shape_corner_br = false;
  bool have_shape_intrinsics = false;
  bool have_shape_angles = false;

  Eigen::Quaternionf orientation = Eigen::Quaternionf::Identity();
  Eigen::Vector3f position = Eigen::Vector3f::Zero();

  std::shared_ptr<Geometry> geometry;
};

GeometryCorrectedProjectorView::GeometryCorrectedProjectorView()
  : _impl(std::make_unique<Impl>()) {}

GeometryCorrectedProjectorView::~GeometryCorrectedProjectorView() {}

void GeometryCorrectedProjectorView::setBufferWidth(int W) {
  _impl->buffer_width = W;
}

void GeometryCorrectedProjectorView::setBufferHeight(int H) {
  _impl->buffer_height = H;
}

void GeometryCorrectedProjectorView::setTopLeftCorner(gmTypes::float3 tlc) {
  _impl->shape_corner_tl = Eigen::Vector3f(tlc[0], tlc[1], tlc[2]);
  _impl->have_shape_corner_tl = true;
}

void GeometryCorrectedProjectorView::setBottomRightCorner(gmTypes::float3 brc) {
  _impl->shape_corner_br = Eigen::Vector3f(brc[0], brc[1], brc[2]);
  _impl->have_shape_corner_br = true;
}

void GeometryCorrectedProjectorView::setPosition(gmTypes::float3 p) {
  _impl->position = Eigen::Vector3f(p[0], p[1], p[2]);
}

void GeometryCorrectedProjectorView::setExtrinsics(gmTypes::float12 M) {
  _impl->position = Eigen::Vector3f(M[3], M[7], M[11]);
  Eigen::Matrix3f eM;
  eM <<
    M[0], M[1], M[2],
    M[4], M[5], M[6],
    M[8], M[9], M[10];
  _impl->orientation = Eigen::Quaternionf(eM);
}

void GeometryCorrectedProjectorView::setIntrinsics(gmTypes::float4 m) {
  _impl->shape_intrinsics = m;
  _impl->have_shape_intrinsics = true;
}

void GeometryCorrectedProjectorView::setAngles(gmTypes::float4 a) {
  _impl->shape_angles = a;
  _impl->have_shape_angles = true;
}

void GeometryCorrectedProjectorView::setQuaternion(gmTypes::float4 q) {
  _impl->orientation = Eigen::Quaternionf(q[0], q[1], q[2], q[3]);
}

void GeometryCorrectedProjectorView::setAxisAngle(gmTypes::float4 aa) {
  Eigen::Quaternionf Q(Eigen::Quaternionf::AngleAxisType
                       (aa[3], Eigen::Vector3f(aa[0], aa[1], aa[2]).normalized()));
  _impl->orientation = Q;
}

void GeometryCorrectedProjectorView::setEulerAngles(gmTypes::float3 ea) {
  Eigen::Quaternionf Q(Eigen::AngleAxisf(ea[0], Eigen::Vector3f::UnitX()) *
                       Eigen::AngleAxisf(ea[1], Eigen::Vector3f::UnitY()) *
                       Eigen::AngleAxisf(ea[2], Eigen::Vector3f::UnitZ()));
  _impl->orientation = Q;
}

void GeometryCorrectedProjectorView::setGeometry(std::shared_ptr<Geometry> g) {
  _impl->geometry = g;
}

void GeometryCorrectedProjectorView::renderFullPipeline(ViewSettings settings, Eye eye) {
  _impl->renderFullPipeline(settings, eye);
}

bool GeometryCorrectedProjectorView::Impl::setCamera(Camera &c) {

  c.setPose(position, orientation);

  if (have_shape_intrinsics) {

    if (have_shape_corner_tl ||
        have_shape_corner_br ||
        have_shape_angles)
      GM_RUNONCE(GM_WRN("GeometryCorrectedProjectorView", "Projection shape set in multiple ways - either intrinsics, angles or corners required. Using intrinsics."));

    return setCameraShapeFromIntrinsics(c);
  }
  else if (have_shape_angles) {

    if (have_shape_corner_tl ||
        have_shape_corner_br)
      GM_RUNONCE(GM_WRN("GeometryCorrectedProjectorView", "Projection shape set in multiple ways - either intrinsics, angles or corners required. Using angles."));

    return setCameraShapeFromAngles(c);
  }
  else if (have_shape_corner_tl ||
           have_shape_corner_br) {

    if (!(have_shape_corner_tl &&
          have_shape_corner_br)) {
      GM_RUNONCE(GM_ERR("GeometryCorrectedProjectorView", "Projection shape by corners incorrectly set - missing parameter."));
      return false;
    }

    return setCameraShapeFromCorners(c);
  }

  GM_RUNONCE(GM_ERR("GeometryCorrectedProjectorView", "Missing parameter for projection shape."));
  return false;
}

bool GeometryCorrectedProjectorView::Impl::setCameraShapeFromIntrinsics(Camera &c) {
  c.setPlanes((1.f - shape_intrinsics[2]) / shape_intrinsics[0],
              (      shape_intrinsics[2]) / shape_intrinsics[0],
              (1.f - shape_intrinsics[3]) / shape_intrinsics[1],
              (      shape_intrinsics[3]) / shape_intrinsics[1]);
  return true;
}

bool GeometryCorrectedProjectorView::Impl::setCameraShapeFromAngles(Camera &c) {
  c.setAngles(shape_angles[0],
              shape_angles[1],
              shape_angles[2],
              shape_angles[3]);
  return true;
}

bool GeometryCorrectedProjectorView::Impl::setCameraShapeFromCorners(Camera &c) {

  Eigen::Vector3f tl = orientation * (shape_corner_tl - position);
  Eigen::Vector3f br = orientation * (shape_corner_br - position);

  c.setPlanes(tl[0] / tl[2],
              br[0] / br[2],
              br[1] / br[2],
              tl[1] / tl[2]);

  return true;
}

std::string GeometryCorrectedProjectorView::Impl::createFragmentCode() {
  // TODO: put together fragment code with the mapper from geometry
  return "";
}

void GeometryCorrectedProjectorView::Impl::renderFullPipeline(ViewSettings settings, Eye eye) {

  if (!geometry) {
    GM_RUNONCE(GM_ERR("GeometryCorrectedProjectorView", "Missing geometry, that is necessary to render the geometry corrected view."));
    return;
  }

  Camera projector_camera;
  if (!setCamera(projector_camera))
    return;

  Eigen::Vector3f render_position = settings.viewpoint
    ? settings.viewpoint->getPosition()
    : Eigen::Vector3f::Zero();

  Camera render_camera;
  bool camera_good =
    geometry->getCameraFromPosition(projector_camera,
                                    render_position,
                                    render_camera);
  if (!camera_good) return;

  if (!is_setup) {
    is_setup = true;
    raster_processor.setFragmentCode(createFragmentCode());
    if (render_target.init() && raster_processor.init()) is_functional = true;
  }

  if (!is_functional) {
    GM_RUNONCE(GM_ERR("GeometryCorrectedProjectorView", "Dysfunctional internal GL workings."));
    return;
  }    

  // Render all renderers to the offscreen buffer

  render_target.push();
  render_target.bind(buffer_width, buffer_height);

  for (auto renderer : settings.renderers)
    renderer->render(render_camera);

  render_target.pop();

  // Render offscreen buffer to previously active render target

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, render_target.getTexId());

  glUseProgram(raster_processor.getProgramId());
  geometry->setMapperUniforms(raster_processor.getProgramId());

  raster_processor.run();

  glBindTexture(GL_TEXTURE_2D, 0);
}

END_NAMESPACE_GMGRAPHICS;
