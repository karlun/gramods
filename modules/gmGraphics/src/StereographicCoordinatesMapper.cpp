
#include <gmGraphics/StereographicCoordinatesMapper.hh>

#include <cmath>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE(StereographicCoordinatesMapper);
GM_OFI_PARAM(StereographicCoordinatesMapper, radius, float, StereographicCoordinatesMapper::setRadius);
GM_OFI_PARAM(StereographicCoordinatesMapper, coverageAngle, gmCore::angle, StereographicCoordinatesMapper::setCoverageAngle);
GM_OFI_PARAM(StereographicCoordinatesMapper, theta0, gmCore::angle, StereographicCoordinatesMapper::setTheta0);
GM_OFI_PARAM(StereographicCoordinatesMapper, phi0, gmCore::angle, StereographicCoordinatesMapper::setPhi0);

struct StereographicCoordinatesMapper::Impl {
  float radius = 0.5;
  gmCore::angle phi_0 = GM_PI_2;
  gmCore::angle theta_0 = 0;
};

StereographicCoordinatesMapper::StereographicCoordinatesMapper()
  : _impl(std::make_unique<Impl>()) {}
StereographicCoordinatesMapper::~StereographicCoordinatesMapper() {}

std::string StereographicCoordinatesMapper::getMapperCode() {
  static const std::string code = R"lang=glsl(

uniform float R;
uniform float theta_0;
uniform float phi_0;

#define PI2 1.57079632679489661923132169163975144209858469968755

bool mapTo2D(vec3 pos3, out vec2 pos2) {

  float r = sqrt(dot(pos3.xz, pos3.xz));

  float phi = atan(pos3.y, r);
  float theta = atan(pos3.x, -pos3.z);

  float k = 2 * R / (1 + sin(phi_0) * sin(phi) + cos(phi_0) * cos(phi) * cos(theta - theta_0));

  float x = k * cos(phi) * sin(theta - theta_0);
  float y = k * (cos(phi_0) * sin(phi) -
                 sin(phi_0) * cos(phi) * cos(theta - theta_0));

  if (x < -1 || x > 1 || y < -1 || y > 1) return false;

  pos2 = vec2(x, y);
  return true;
}

bool mapTo3D(vec2 pos2, out vec3 pos3) {

  float r = sqrt(dot(pos2, pos2));
  float c = 2 * atan(r/(2*R));

  float phi = asin(cos(c) * sin(phi_0) + (pos2.y * sin(c) * cos(phi_0))/r);
  float theta = theta_0 + atan((pos2.x * sin(c)),
                               (r * cos(phi_0) * cos(c) - pos2.y * sin(phi_0) * sin(c)));

  pos3 = vec3(cos(phi) * sin(theta), sin(phi), -cos(phi) * cos(theta));
  return true;
}
)lang=glsl";
  return code;
}

void StereographicCoordinatesMapper::setMapperUniforms(GLuint program_id) {
  glUniform1f(glGetUniformLocation(program_id, "R"), _impl->radius);
  glUniform1f(glGetUniformLocation(program_id, "theta_0"), _impl->theta_0);
  glUniform1f(glGetUniformLocation(program_id, "phi_0"), _impl->phi_0);
}

void StereographicCoordinatesMapper::setRadius(float R) {
  if (R < std::numeric_limits<float>::epsilon())
    throw gmCore::InvalidArgument(GM_STR("Invalid sphere radius " << R << " for stereographic projection"));
  _impl->radius = R;
}

void StereographicCoordinatesMapper::setCoverageAngle(gmCore::angle a) {
  double angle = 0.5 * (double) a;
  if (angle < std::numeric_limits<double>::epsilon() ||
      angle > GM_2_PI - std::numeric_limits<double>::epsilon())
    throw gmCore::InvalidArgument(GM_STR("Invalid coverage angle " << a << " for stereographic projection"));
  _impl->radius = (float)( (1.0 + std::cos(angle)) / (2.0 * std::sin(angle)) );
}

void StereographicCoordinatesMapper::setTheta0(gmCore::angle a) {
  _impl->theta_0 = a;
}

void StereographicCoordinatesMapper::setPhi0(gmCore::angle a) {
  _impl->phi_0 = a;
}

END_NAMESPACE_GMGRAPHICS;
