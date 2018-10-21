
#include <gmGraphics/AngularFisheyeSpatialDomeView.hh>

#include <gmGraphics/CubeMap.hh>
#include <gmGraphics/GLUtils.hh>

#include <GL/glew.h>
#include <GL/gl.h>

#include <vector>
#include <Eigen/Eigen>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE_SUB(AngularFisheyeSpatialDomeView, StereoscopicView);
GM_OFI_PARAM(AngularFisheyeSpatialDomeView, cubeMapResolution, int, AngularFisheyeSpatialDomeView::setCubeMapResolution);
GM_OFI_PARAM(AngularFisheyeSpatialDomeView, linearInterpolation, bool, AngularFisheyeSpatialDomeView::setLinearInterpolation);
GM_OFI_PARAM(AngularFisheyeSpatialDomeView, coverageAngle, float, AngularFisheyeSpatialDomeView::setCoverageAngle);
GM_OFI_PARAM(AngularFisheyeSpatialDomeView, domePosition, gmTypes::float3, AngularFisheyeSpatialDomeView::setDomePosition);
GM_OFI_PARAM(AngularFisheyeSpatialDomeView, domeRadius, float, AngularFisheyeSpatialDomeView::setDomeRadius);

struct AngularFisheyeSpatialDomeView::Impl {

  static const std::string fragment_code;
  float coverage_angle = gramods_PI;

  Eigen::Vector3f dome_position = Eigen::Vector3f::Zero();
  float dome_radius = 10;
  float eye_separation = 0;

  std::unique_ptr<CubeMap> cubemap;

  Impl() : cubemap(std::make_unique<CubeMap>()) {
    cubemap->setFragmentCode(fragment_code);
  }

  void renderFullPipeline(ViewSettings settings, Eye eye);

};

const std::string AngularFisheyeSpatialDomeView::Impl::fragment_code = R"(
#version 330 core

uniform sampler2D texLeft;
uniform sampler2D texRight;
uniform sampler2D texBottom;
uniform sampler2D texTop;
uniform sampler2D texBack;
uniform sampler2D texFront;

uniform float coverageAngle;
uniform vec3 eye_position;
uniform float dome_radius;
uniform float cubemap_radius;

in vec2 pos;
out vec4 fragColor;

void colorFromTex(float x, float y, float z, sampler2D tex) {
  fragColor = vec4(texture(tex, 0.5 * vec2(x/z, y/z) + 0.5).rgb, 1);
}

vec3 tex_coord_direction(vec3 line) {
  vec3 pos_on_sphere = dome_radius * normalize(line);
  return normalize(pos_on_sphere - eye_position);
}

float plane_line_d(vec3 line, vec3 n) {
  if (dot(line, n) <= 0) return 1e10;
  vec3 plane_position = cubemap_radius * n;
  float d = dot(plane_position - eye_position, n) / dot(line, n);
  return d;
}

void main() {

  float r = sqrt(dot(pos.xy, pos.xy));
  if (r > 1) {
    fragColor = vec4(0, 0, 0, 1);
    return;
  }

  float phi = 0.5 * r * coverageAngle;
  float theta = atan(pos.y, pos.x);

  vec3 pix_dir = vec3(cos(theta) * sin(phi), cos(phi), sin(theta) * sin(phi));
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
)";

AngularFisheyeSpatialDomeView::AngularFisheyeSpatialDomeView()
  : _impl(std::make_unique<Impl>()) {}

void AngularFisheyeSpatialDomeView::renderFullPipeline(ViewSettings settings, Eye eye) {
  populateViewSettings(settings);
  _impl->eye_separation = eye_separation;
  _impl->renderFullPipeline(settings, eye);
}

void AngularFisheyeSpatialDomeView::Impl::renderFullPipeline(ViewSettings settings, Eye eye) {

  Eigen::Vector3f pos = Eigen::Vector3f::Zero();
  Eigen::Quaternionf rot = Eigen::Quaternionf::Identity();

  if (settings.viewpoint) {
    pos = settings.viewpoint->getPosition();
    rot = settings.viewpoint->getOrientation();
  }

  switch (eye) {
  case Eye::LEFT:
    pos -= rot * Eigen::Vector3f(0.5f * eye_separation, 0.f, 0.f);
    break;
  case Eye::RIGHT:
    pos += rot * Eigen::Vector3f(0.5f * eye_separation, 0.f, 0.f);
  }

  GLint program_id = cubemap->getProgram();
  if (program_id) {
    glUseProgram(program_id);
    glUniform1f(glGetUniformLocation(program_id, "coverageAngle"), coverage_angle);
    glUniform3fv(glGetUniformLocation(program_id, "eye_position"), 1, pos.data());
    glUniform1f(glGetUniformLocation(program_id, "dome_radius"), dome_radius);
    glUniform1f(glGetUniformLocation(program_id, "cubemap_radius"), dome_radius);
  }

  cubemap->setSpatialCubeMap(dome_position, 2 * dome_radius);

  cubemap->renderFullPipeline(settings.renderers, pos, rot);
}

void AngularFisheyeSpatialDomeView::setCubeMapResolution(int res) {
  _impl->cubemap->setCubeMapResolution(res);
}

void AngularFisheyeSpatialDomeView::setLinearInterpolation(bool on) {
  _impl->cubemap->setLinearInterpolation(on);
}

void AngularFisheyeSpatialDomeView::setCoverageAngle(float a) {
  _impl->coverage_angle = a;
}

void AngularFisheyeSpatialDomeView::setDomePosition(gmTypes::float3 c) {
  assert(0);
  _impl->dome_position = Eigen::Vector3f(c[0], c[1], c[2]);
}

void AngularFisheyeSpatialDomeView::setDomeRadius(float r) {
  _impl->dome_radius = r;
}


END_NAMESPACE_GMGRAPHICS;
