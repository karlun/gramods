
#include <gmGraphics/GeometryCorrectedProjectorView.hh>

#include <gmGraphics/GLUtils.hh>
#include <gmCore/RunOnce.hh>

#include <gmGraphics/OffscreenRenderTargets.hh>
#include <gmGraphics/RasterProcessor.hh>

#include <Eigen/Eigen>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE_SUB(GeometryCorrectedProjectorView, StereoscopicView);
GM_OFI_PARAM(GeometryCorrectedProjectorView, bufferWidth, int, GeometryCorrectedProjectorView::setBufferWidth);
GM_OFI_PARAM(GeometryCorrectedProjectorView, bufferHeight, int, GeometryCorrectedProjectorView::setBufferHeight);
GM_OFI_PARAM(GeometryCorrectedProjectorView, linearInterpolation, bool, GeometryCorrectedProjectorView::setLinearInterpolation);
GM_OFI_PARAM(GeometryCorrectedProjectorView, topLeftCorner, gmTypes::float3, GeometryCorrectedProjectorView::setTopLeftCorner);
GM_OFI_PARAM(GeometryCorrectedProjectorView, bottomRightCorner, gmTypes::float3, GeometryCorrectedProjectorView::setBottomRightCorner);
GM_OFI_PARAM(GeometryCorrectedProjectorView, position, gmTypes::float3, GeometryCorrectedProjectorView::setPosition);
GM_OFI_PARAM(GeometryCorrectedProjectorView, extrinsics, gmTypes::float12, GeometryCorrectedProjectorView::setExtrinsics);
GM_OFI_PARAM(GeometryCorrectedProjectorView, intrinsics, gmTypes::float4, GeometryCorrectedProjectorView::setIntrinsics);
GM_OFI_PARAM(GeometryCorrectedProjectorView, clipAngles, gmTypes::float4, GeometryCorrectedProjectorView::setClipAngles);
GM_OFI_PARAM(GeometryCorrectedProjectorView, quaternion, gmTypes::float4, GeometryCorrectedProjectorView::setQuaternion);
GM_OFI_PARAM(GeometryCorrectedProjectorView, axisAngle, gmTypes::float4, GeometryCorrectedProjectorView::setAxisAngle);
GM_OFI_PARAM(GeometryCorrectedProjectorView, eulerAngles, gmTypes::float3, GeometryCorrectedProjectorView::setEulerAngles);
GM_OFI_POINTER(GeometryCorrectedProjectorView, geometry, Geometry, GeometryCorrectedProjectorView::setGeometry);

struct GeometryCorrectedProjectorView::Impl {

  static const std::string fragment_template_code;
  static const std::string mapper_pattern;

  OffscreenRenderTargets render_target;
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

const std::string
GeometryCorrectedProjectorView::
Impl::mapper_pattern = "MAPPER;";

const std::string
GeometryCorrectedProjectorView::
Impl::fragment_template_code = R"lang=glsl(
#version 330 core

MAPPER;

// vec3 getIntersection(vec3 pos, vec3 dir)

uniform sampler2D tex;

uniform mat4 pPV_inv;
uniform mat4 rPV;

in vec2 position;
out vec4 fragColor;

void main() {

  // projector raster near-point and far-point to projection point
  vec4 p0_4 = pPV_inv * vec4(position, -1, 1);
  vec3 p0 = p0_4.xyz / p0_4.w;

  vec4 p1_4 = pPV_inv * vec4(position, 1, 1);
  vec3 p1 = p1_4.xyz / p1_4.w;

  vec3 dir = normalize(p1 - p0);
  vec3 p = getIntersection(p0, dir);

  if (p.x == 0 && p.y == 0 && p.z == 0) {
    fragColor = vec4(0, 0, 0, 0);
    return;
  }

  // projection point to render raster coordinate
  vec4 t = rPV * vec4(p, 1);
  vec2 uv = (t.xy / t.w) * 0.5 + vec2(0.5, 0.5);

  if (uv.x >= 0 && uv.x <= 1 &&
      uv.y >= 0 && uv.y <= 1)
    fragColor = vec4(texture(tex, uv).rgb, 1);
  else
    fragColor = vec4(0, 0, 0, 0);
}
)lang=glsl";

GeometryCorrectedProjectorView::GeometryCorrectedProjectorView()
  : _impl(std::make_unique<Impl>()) {}

GeometryCorrectedProjectorView::~GeometryCorrectedProjectorView() {}

void GeometryCorrectedProjectorView::setBufferWidth(int W) {
  _impl->buffer_width = W;
}

void GeometryCorrectedProjectorView::setBufferHeight(int H) {
  _impl->buffer_height = H;
}

void GeometryCorrectedProjectorView::setLinearInterpolation(bool on) {
  _impl->render_target.setLinearInterpolation(on);
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

void GeometryCorrectedProjectorView::setClipAngles(gmTypes::float4 a) {
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
  c.setClipPlanes((1.f - shape_intrinsics[2]) / shape_intrinsics[0],
                  (      shape_intrinsics[2]) / shape_intrinsics[0],
                  (1.f - shape_intrinsics[3]) / shape_intrinsics[1],
                  (      shape_intrinsics[3]) / shape_intrinsics[1]);
  return true;
}

bool GeometryCorrectedProjectorView::Impl::setCameraShapeFromAngles(Camera &c) {
  c.setClipAngles(shape_angles[0],
                  shape_angles[1],
                  shape_angles[2],
                  shape_angles[3]);
  return true;
}

bool GeometryCorrectedProjectorView::Impl::setCameraShapeFromCorners(Camera &c) {

  Eigen::Vector3f tl = orientation.conjugate() * (shape_corner_tl - position);
  Eigen::Vector3f br = orientation.conjugate() * (shape_corner_br - position);

  c.setClipPlanes(tl[0] / tl[2],
                  br[0] / br[2],
                  br[1] / br[2],
                  tl[1] / tl[2]);

  return true;
}

std::string GeometryCorrectedProjectorView::Impl::createFragmentCode() {
  assert(geometry);
  assert(fragment_template_code.find(mapper_pattern) != std::string::npos);

  std::string mapper_code = geometry->getMapperCode();
  std::string fragment_code = fragment_template_code;

  fragment_code.replace(fragment_code.find(mapper_pattern),
                        mapper_pattern.length(),
                        mapper_code);

  return fragment_code;
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
  GM_VINF("GeometryCorrectedProjectorView", "Render all renderers to the offscreen buffer.");

  render_target.push();
  render_target.bind(buffer_width, buffer_height);

  for (auto renderer : settings.renderers)
    renderer->render(render_camera);

  render_target.pop();

  // Render offscreen buffer to active render target
  GM_VINF("GeometryCorrectedProjectorView", "Render offscreen buffer to active render target");

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, render_target.getTexId());

  GLuint program_id = raster_processor.getProgramId();
  glUseProgram(program_id);

  geometry->setMapperUniforms(program_id);

  glUniform1i(glGetUniformLocation(program_id, "tex"), 0);
  Eigen::Matrix4f pPV =
    projector_camera.getProjectionMatrix(1, 2) *
    projector_camera.getViewMatrix().matrix();
  Eigen::Matrix4f pPV_inv = pPV.inverse();
  glUniformMatrix4fv(glGetUniformLocation(program_id, "pPV_inv"), 1, false, pPV_inv.data());

  Eigen::Matrix4f rPV =
    render_camera.getProjectionMatrix(1, 2) *
    render_camera.getViewMatrix().matrix();
  glUniformMatrix4fv(glGetUniformLocation(program_id, "rPV"), 1, false, rPV.data());

  raster_processor.run();

  glBindTexture(GL_TEXTURE_2D, 0);
}

END_NAMESPACE_GMGRAPHICS;
