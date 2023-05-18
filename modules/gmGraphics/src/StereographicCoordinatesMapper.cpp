
#include <gmGraphics/StereographicCoordinatesMapper.hh>

#include <cmath>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE(StereographicCoordinatesMapper);
GM_OFI_PARAM2(StereographicCoordinatesMapper, radius, float, setRadius);
GM_OFI_PARAM2(StereographicCoordinatesMapper, coverageAngle, gmCore::angle, setCoverageAngle);
GM_OFI_PARAM2(StereographicCoordinatesMapper, theta0, gmCore::angle, setTheta0);
GM_OFI_PARAM2(StereographicCoordinatesMapper, phi0, gmCore::angle, setPhi0);

struct StereographicCoordinatesMapper::Impl {
  float radius = 0.5;
  gmCore::angle phi0 = float(GM_PI_2);
  gmCore::angle theta0 = 0;

  struct uniforms {
    GLint R = 0, theta0 = 0, phi0 = 0;
  };

  std::unordered_map<GLint, uniforms> loc;
};

StereographicCoordinatesMapper::StereographicCoordinatesMapper()
  : _impl(std::make_unique<Impl>()) {}
StereographicCoordinatesMapper::~StereographicCoordinatesMapper() {}

std::string StereographicCoordinatesMapper::getCommonCode() {
  return withVarId(R"lang=glsl(
uniform float ID_R;
uniform float ID_theta0;
uniform float ID_phi0;

#ifndef PI2
#define PI2 1.57079632679489661923132169163975144209858469968755
#endif
)lang=glsl");
}

std::string StereographicCoordinatesMapper::getTo2DCode() {
  return withVarId(R"lang=glsl(
bool mapTo2D(vec3 pos3, out vec2 pos2) {

  float r = sqrt(dot(pos3.xz, pos3.xz));

  float phi = atan(pos3.y, r);
  float theta = atan(pos3.x, -pos3.z);

  float k = 2 * ID_R / (1 + sin(ID_phi0) * sin(phi) + cos(ID_phi0) * cos(phi) * cos(theta - ID_theta0));

  float x = k * cos(phi) * sin(theta - ID_theta0);
  float y = k * (cos(ID_phi0) * sin(phi) -
                 sin(ID_phi0) * cos(phi) * cos(theta - ID_theta0));

  if (x < -1 || x > 1 || y < -1 || y > 1) return false;

  pos2 = vec2(x, y);
  return true;
}
)lang=glsl");
}

std::string StereographicCoordinatesMapper::getTo3DCode() {
  return withVarId(R"lang=glsl(
bool mapTo3D(vec2 pos2, out vec3 pos3) {

  float r = sqrt(dot(pos2, pos2));
  float c = 2 * atan(r/(2*ID_R));

  float phi = asin(cos(c) * sin(ID_phi0) + (pos2.y * sin(c) * cos(ID_phi0))/r);
  float theta = ID_theta0 + atan((pos2.x * sin(c)),
                               (r * cos(ID_phi0) * cos(c) - pos2.y * sin(ID_phi0) * sin(c)));

  pos3 = vec3(cos(phi) * sin(theta), sin(phi), -cos(phi) * cos(theta));
  return true;
}
)lang=glsl");
}

#define LOC(VAR, NAME)                                                         \
  (_impl->loc[program_id].VAR > 0                                              \
       ? _impl->loc[program_id].VAR                                            \
       : (_impl->loc[program_id].VAR =                                         \
              glGetUniformLocation(program_id, withVarId(NAME).c_str())))

void StereographicCoordinatesMapper::setCommonUniforms(GLuint program_id) {
  glUniform1f(LOC(R, "ID_R"), _impl->radius);
  glUniform1f(LOC(theta0, "ID_theta0"), _impl->theta0);
  glUniform1f(LOC(phi0, "ID_phi0"), _impl->phi0);
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
  _impl->theta0 = a;
}

void StereographicCoordinatesMapper::setPhi0(gmCore::angle a) {
  _impl->phi0 = a;
}

END_NAMESPACE_GMGRAPHICS;
