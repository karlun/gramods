
#include <gmGraphics/SpatialSphericalView.hh>

#include <gmGraphics/CubeMap.hh>
#include <gmGraphics/GLUtils.hh>
#include <gmCore/RunOnce.hh>

#include <GL/glew.h>
#include <GL/gl.h>

#include <vector>
#include <Eigen/Eigen>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE_SUB(SpatialSphericalView, StereoscopicView);
GM_OFI_PARAM(SpatialSphericalView, cubeMapResolution, int, SpatialSphericalView::setCubeMapResolution);
GM_OFI_PARAM(SpatialSphericalView, linearInterpolation, bool, SpatialSphericalView::setLinearInterpolation);
GM_OFI_PARAM(SpatialSphericalView, makeSquare, bool, SpatialSphericalView::setMakeSquare);
GM_OFI_POINTER(SpatialSphericalView, coordinatesMapper, gmGraphics::CoordinatesMapper, SpatialSphericalView::setCoordinatesMapper);
GM_OFI_PARAM(SpatialSphericalView, sphereCenter, Eigen::Vector3f, SpatialSphericalView::setSphereCenter);
GM_OFI_PARAM(SpatialSphericalView, sphereRadius, float, SpatialSphericalView::setSphereRadius);
GM_OFI_PARAM(SpatialSphericalView, tiltAngle, float, SpatialSphericalView::setTiltAngle);

struct SpatialSphericalView::Impl {

  static const std::string fragment_template_code;
  static const std::string mapper_pattern;

  bool make_square = false;
  Eigen::Vector3f sphere_center = Eigen::Vector3f::Zero();
  float sphere_radius = 10;
  float tilt_angle = 0;
  float eye_separation = 0;

  std::unique_ptr<CubeMap> cubemap;
  std::shared_ptr<CoordinatesMapper> mapper;

  Impl() : cubemap(std::make_unique<CubeMap>()) {}

  std::string createFragmentCode();
  void renderFullPipeline(ViewSettings settings, Eye eye);

};

const std::string SpatialSphericalView::Impl::mapper_pattern = "MAPPER;";

const std::string SpatialSphericalView::Impl::fragment_template_code = R"lang=glsl(
#version 330 core

uniform sampler2D texLeft;
uniform sampler2D texRight;
uniform sampler2D texBottom;
uniform sampler2D texTop;
uniform sampler2D texBack;
uniform sampler2D texFront;

uniform vec3 eye_position;
uniform float radius;
uniform float cubemap_radius;

in vec2 pos;
out vec4 fragColor;

MAPPER;

void colorFromTex(float x, float y, float z, sampler2D tex) {
  fragColor = texture(tex, 0.5 * vec2(x/z, y/z) + 0.5);
}

vec3 tex_coord_direction(vec3 line) {
  vec3 pos_on_sphere = radius * normalize(line);
  return normalize(pos_on_sphere - eye_position);
}

float plane_line_d(vec3 line, vec3 n) {
  if (dot(line, n) <= 0) return 1e10;
  vec3 plane_position = cubemap_radius * n;
  float d = dot(plane_position - eye_position, n) / dot(line, n);
  return d;
}

void main() {

  vec3 pix_dir;
  bool good = mapTo3D(pos, pix_dir);

  if (!good) {
    fragColor = vec4(0, 0, 0, 0);
    return;
  }

  vec3 tex_dir = tex_coord_direction(pix_dir);

  float d_left = plane_line_d(tex_dir, vec3(-1, 0, 0));
  float d_right = plane_line_d(tex_dir, vec3(1, 0, 0));
  float d_bottom = plane_line_d(tex_dir, vec3(0, -1, 0));
  float d_top = plane_line_d(tex_dir, vec3(0, 1, 0));
  float d_back = plane_line_d(tex_dir, vec3(0, 0, 1));
  float d_front = plane_line_d(tex_dir, vec3(0, 0, -1));

  float min_d = min(min(min(d_left, d_right), min(d_bottom, d_top)), min(d_back, d_front));
  vec3 pix = eye_position + min_d * tex_dir;

  if (min_d == d_left) {
    colorFromTex(-pix.z,  pix.y, -pix.x, texLeft);
    return;
  }

  if (min_d == d_right) {
    colorFromTex( pix.z,  pix.y,  pix.x, texRight);
    return;
  }

  if (min_d == d_bottom) {
    colorFromTex( pix.x, -pix.z, -pix.y, texBottom);
    return;
  }

  if (min_d == d_top) {
    colorFromTex( pix.x,  pix.z,  pix.y, texTop);
    return;
  }

  if (min_d == d_back) {
    colorFromTex(-pix.x,  pix.y,  pix.z, texBack);
    return;
  }

  if (min_d == d_front) {
    colorFromTex( pix.x,  pix.y, -pix.z, texFront);
    return;
  }

  fragColor = vec4(0.6, 0.3, 0.1, 1);
}
)lang=glsl";

SpatialSphericalView::SpatialSphericalView()
  : _impl(std::make_unique<Impl>()) {}

void SpatialSphericalView::renderFullPipeline(ViewSettings settings, Eye eye) {
  populateViewSettings(settings);
  _impl->eye_separation = eye_separation;
  _impl->renderFullPipeline(settings, eye);
}

void SpatialSphericalView::Impl::renderFullPipeline(ViewSettings settings, Eye eye) {

  if (!mapper) {
    GM_RUNONCE(GM_ERR("SpatialSphericalView", "No coordinate mapper specified."));
    return;
  }

  Eigen::Vector3f eye_pos = Eigen::Vector3f::Zero();
  Eigen::Quaternionf head_rot = Eigen::Quaternionf::Identity();
  Eigen::Quaternionf tilt
    (Eigen::AngleAxis<float>(tilt_angle, Eigen::Vector3f(-1, 0, 0)));

  if (settings.viewpoint) {
    eye_pos = settings.viewpoint->getPosition();
    head_rot = settings.viewpoint->getOrientation();
  }

  switch (eye) {
  case Eye::LEFT:
    eye_pos -= head_rot * Eigen::Vector3f(0.5f * eye_separation, 0.f, 0.f);
    break;
  case Eye::RIGHT:
    eye_pos += head_rot * Eigen::Vector3f(0.5f * eye_separation, 0.f, 0.f);
  }

  GLint program_id = cubemap->getProgram();
  if (!program_id) {

    cubemap->setFragmentCode(createFragmentCode());

    std::vector<std::shared_ptr<Renderer>> no_renderers;
    cubemap->renderFullPipeline(no_renderers, eye_pos, tilt, make_square);
    program_id = cubemap->getProgram();

    if (!program_id) {
      GM_RUNONCE(GM_WRN("SpatialSphericalView", "Could not initialize cubemap"));
      return;
    }
  }

  Eigen::Vector3f offset = eye_pos - sphere_center;

  glUseProgram(program_id);
  glUniform3fv(glGetUniformLocation(program_id, "eye_position"), 1, offset.data());
  glUniform1f(glGetUniformLocation(program_id, "radius"), sphere_radius);
  glUniform1f(glGetUniformLocation(program_id, "cubemap_radius"), sphere_radius);
  mapper->setMapperUniforms(program_id);
  glUseProgram(0);

  cubemap->setSpatialCubeMap(sphere_center, 2 * sphere_radius);
  cubemap->renderFullPipeline(settings.renderers, eye_pos, tilt, make_square);
}

std::string SpatialSphericalView::Impl::createFragmentCode() {
  assert(mapper);
  assert(fragment_template_code.find(mapper_pattern) != std::string::npos);

  std::string mapper_code = mapper->getMapperCode();
  std::string fragment_code = fragment_template_code;

  fragment_code.replace(fragment_code.find(mapper_pattern),
                        mapper_pattern.length(),
                        mapper_code);

  return fragment_code;
}

void SpatialSphericalView::setCoordinatesMapper(std::shared_ptr<CoordinatesMapper> mapper) {
  _impl->mapper = mapper;
}

void SpatialSphericalView::setMakeSquare(bool on) {
  _impl->make_square = on;
}

void SpatialSphericalView::setCubeMapResolution(int res) {
  _impl->cubemap->setCubeMapResolution(res);
}

void SpatialSphericalView::setLinearInterpolation(bool on) {
  _impl->cubemap->setLinearInterpolation(on);
}

void SpatialSphericalView::setSphereCenter(Eigen::Vector3f c) {
  assert(0);
  _impl->sphere_center = c;
}

void SpatialSphericalView::setSphereRadius(float r) {
  _impl->sphere_radius = r;
}

void SpatialSphericalView::setTiltAngle(float a) {
  _impl->tilt_angle = a;
}

END_NAMESPACE_GMGRAPHICS;
