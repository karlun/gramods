
#include <gmGraphics/GeometryCorrectedProjectorView.hh>

#include <gmGraphics/GLUtils.hh>
#include <gmCore/RunOnce.hh>

#include <gmGraphics/OffscreenRenderTargets.hh>
#include <gmGraphics/RasterProcessor.hh>

#include <Eigen/Eigen>

#include <optional>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE_SUB(GeometryCorrectedProjectorView, MultiscopicView);
GM_OFI_PARAM2(GeometryCorrectedProjectorView, bufferWidth, int, setBufferWidth);
GM_OFI_PARAM2(GeometryCorrectedProjectorView, bufferHeight, int, setBufferHeight);
GM_OFI_PARAM2(GeometryCorrectedProjectorView, linearInterpolation, bool, setLinearInterpolation);
GM_OFI_PARAM2(GeometryCorrectedProjectorView, topLeftCorner, Eigen::Vector3f, setTopLeftCorner);
GM_OFI_PARAM2(GeometryCorrectedProjectorView, bottomRightCorner, Eigen::Vector3f, setBottomRightCorner);
GM_OFI_PARAM2(GeometryCorrectedProjectorView, position, Eigen::Vector3f, setPosition);
GM_OFI_PARAM2(GeometryCorrectedProjectorView, extrinsics, gmCore::float12, setExtrinsics);
GM_OFI_PARAM2(GeometryCorrectedProjectorView, intrinsics, gmCore::float4, setIntrinsics);
GM_OFI_PARAM2(GeometryCorrectedProjectorView, clipAngles, gmCore::angle4, setClipAngles);
GM_OFI_PARAM2(GeometryCorrectedProjectorView, orientation, Eigen::Quaternionf, setOrientation);
GM_OFI_POINTER2(GeometryCorrectedProjectorView, geometry, Geometry, setGeometry);

struct GeometryCorrectedProjectorView::Impl {

  static const std::string fragment_template_code;
  static const std::string mapper_pattern;

  OffscreenRenderTargets render_target;
  RasterProcessor raster_processor;

  bool is_setup = false;
  bool is_functional = false;

  void renderFullPipeline(ViewSettings settings, Eye eye);
  void renderFullPipeline(ViewSettings settings,
                          Eye eye,
                          Camera *projector_camera,
                          Viewpoint *viewpoint);

  bool setCamera(Camera &c);
  bool setCameraShapeFromIntrinsics(Camera &c);
  bool setCameraShapeFromAngles(Camera &c);
  bool setCameraShapeFromCorners(Camera &c);

  std::string createFragmentCode();

  size_t buffer_width = 2048;
  size_t buffer_height = 2048;

  std::optional<Eigen::Vector3f> shape_corner_tl;
  std::optional<Eigen::Vector3f> shape_corner_br;
  std::optional<gmCore::float4> shape_intrinsics;
  std::optional<gmCore::angle4> shape_angles;

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
    fragColor = texture(tex, uv);
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

void GeometryCorrectedProjectorView::setTopLeftCorner(Eigen::Vector3f tlc) {
  _impl->shape_corner_tl = tlc;
}

void GeometryCorrectedProjectorView::setBottomRightCorner(Eigen::Vector3f brc) {
  _impl->shape_corner_br = brc;
}

void GeometryCorrectedProjectorView::setPosition(Eigen::Vector3f p) {
  _impl->position = p;
}

void GeometryCorrectedProjectorView::setExtrinsics(gmCore::float12 M) {
  _impl->position = Eigen::Vector3f(M[3], M[7], M[11]);
  Eigen::Matrix3f eM;
  eM <<
    M[0], M[1], M[2],
    M[4], M[5], M[6],
    M[8], M[9], M[10];
  _impl->orientation = Eigen::Quaternionf(eM);
}

void GeometryCorrectedProjectorView::setIntrinsics(gmCore::float4 m) {
  _impl->shape_intrinsics = m;
}

void GeometryCorrectedProjectorView::setClipAngles(gmCore::angle4 a) {
  _impl->shape_angles = a;
}

void GeometryCorrectedProjectorView::setOrientation(Eigen::Quaternionf q) {
  _impl->orientation = q;
}

void GeometryCorrectedProjectorView::setGeometry(std::shared_ptr<Geometry> g) {
  _impl->geometry = g;
}

void GeometryCorrectedProjectorView::renderFullPipeline(ViewSettings settings, Eye eye) {
  _impl->renderFullPipeline(settings, eye);
}

bool GeometryCorrectedProjectorView::Impl::setCamera(Camera &c) {

  c.setPose(position, orientation);

  if (shape_intrinsics) {

    if (shape_corner_tl ||
        shape_corner_br ||
        shape_angles)
      GM_RUNONCE(GM_WRN("GeometryCorrectedProjectorView", "Projection shape set in multiple ways - either intrinsics, angles or corners required. Using intrinsics."));

    return setCameraShapeFromIntrinsics(c);
  }
  else if (shape_angles) {

    if (shape_corner_tl ||
        shape_corner_br)
      GM_RUNONCE(GM_WRN("GeometryCorrectedProjectorView", "Projection shape set in multiple ways - either intrinsics, angles or corners required. Using angles."));

    return setCameraShapeFromAngles(c);
  }
  else if (shape_corner_tl ||
           shape_corner_br) {

    if (!(shape_corner_tl &&
          shape_corner_br)) {
      GM_RUNONCE(GM_ERR("GeometryCorrectedProjectorView", "Projection shape by corners incorrectly set - missing parameter."));
      return false;
    }

    return setCameraShapeFromCorners(c);
  }

  GM_RUNONCE(GM_ERR("GeometryCorrectedProjectorView", "Missing parameter for projection shape."));
  return false;
}

bool GeometryCorrectedProjectorView::Impl::setCameraShapeFromIntrinsics(Camera &c) {
  auto &shape_intrinsics = *this->shape_intrinsics;
  c.setClipPlanes((    - shape_intrinsics[2]) / shape_intrinsics[0],
                  (1.f - shape_intrinsics[2]) / shape_intrinsics[0],
                  (    - shape_intrinsics[3]) / shape_intrinsics[1],
                  (1.f - shape_intrinsics[3]) / shape_intrinsics[1]);
  return true;
}

bool GeometryCorrectedProjectorView::Impl::setCameraShapeFromAngles(Camera &c) {
  auto &shape_angles = *this->shape_angles;
  c.setClipAngles(shape_angles[0],
                  shape_angles[1],
                  shape_angles[2],
                  shape_angles[3]);
  return true;
}

bool GeometryCorrectedProjectorView::Impl::setCameraShapeFromCorners(Camera &c) {

  Eigen::Vector3f tl = orientation.conjugate() * (*shape_corner_tl - position);
  Eigen::Vector3f br = orientation.conjugate() * (*shape_corner_br - position);

  c.setClipPlanes(tl[0] / tl[2],
                  br[0] / br[2],
                  br[1] / br[2],
                  tl[1] / tl[2]);

  return true;
}

std::string GeometryCorrectedProjectorView::Impl::createFragmentCode() {
  assert(geometry);
  assert(fragment_template_code.find(mapper_pattern) != std::string::npos);

  std::string mapper_code = geometry->getIntersectionCode();
  std::string fragment_code = fragment_template_code;

  fragment_code.replace(fragment_code.find(mapper_pattern),
                        mapper_pattern.length(),
                        mapper_code);

  return fragment_code;
}

void GeometryCorrectedProjectorView::Impl::renderFullPipeline(ViewSettings settings,
                                                              Eye eye) {

  if (!geometry) {
    GM_RUNONCE(GM_ERR("GeometryCorrectedProjectorView", "Missing geometry, that is necessary to render the geometry corrected view."));
    return;
  }

  Camera projector_camera(settings);
  if (!setCamera(projector_camera))
    return;
  projector_camera.setNearFar(1.f, 2.f);

  for (auto viewpoint : settings.viewpoints)
    renderFullPipeline(settings, eye, &projector_camera, viewpoint.get());
}

void GeometryCorrectedProjectorView::Impl::renderFullPipeline(
    ViewSettings settings,
    Eye eye,
    Camera *projector_camera,
    Viewpoint *viewpoint) {

  Eigen::Vector3f x_VP = viewpoint->getPosition(eye);
  Eigen::Quaternionf q_VP = viewpoint->getOrientation(eye);

  Camera render_camera(settings);
  bool camera_good =
      geometry->getCameraFromPosition(*projector_camera, x_VP, render_camera);
  if (!camera_good) return;

  render_camera.setEye(eye);

  if (!is_setup) {
    is_setup = true;
    raster_processor.setFragmentCode(createFragmentCode());
    render_target.setPixelFormat(settings.pixel_format);
    if (render_target.init() && raster_processor.init()) is_functional = true;
  }

  if (!is_functional) {
    GM_RUNONCE(GM_ERR("GeometryCorrectedProjectorView", "Dysfunctional internal GL workings."));
    return;
  }    

  // Render all renderers to the offscreen buffer
  GM_DBG2("GeometryCorrectedProjectorView", "Render all renderers to the offscreen buffer.");

  render_target.push();
  render_target.bind(buffer_width, buffer_height);

  glEnable(GL_DEPTH_TEST);

  settings.renderNodes(render_camera);

  render_target.pop();

  glDisable(GL_DEPTH_TEST);

  // Render offscreen buffer to active render target
  GM_DBG2("GeometryCorrectedProjectorView", "Render offscreen buffer to active render target");

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, render_target.getTexId());

  GLuint program_id = raster_processor.getProgramId();
  glUseProgram(program_id);

  geometry->setMapperUniforms(program_id);

  glUniform1i(glGetUniformLocation(program_id, "tex"), 0);
  Eigen::Matrix4f pPV = projector_camera->getProjectionMatrix() *
                        projector_camera->getViewMatrix().matrix();
  Eigen::Matrix4f pPV_inv = pPV.inverse();
  glUniformMatrix4fv(glGetUniformLocation(program_id, "pPV_inv"), 1, false, pPV_inv.data());

  render_camera.setNearFar(1.f, 2.f);
  Eigen::Matrix4f rPV =
    render_camera.getProjectionMatrix() *
    render_camera.getViewMatrix().matrix();
  glUniformMatrix4fv(glGetUniformLocation(program_id, "rPV"), 1, false, rPV.data());

  raster_processor.run();

  glBindTexture(GL_TEXTURE_2D, 0);
}

void GeometryCorrectedProjectorView::traverse(Visitor *visitor) {
  MultiscopicView::traverse(visitor);
  if (_impl->geometry) _impl->geometry->accept(visitor);
}

END_NAMESPACE_GMGRAPHICS;
