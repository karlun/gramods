
#include <gmGraphics/AngularFisheyeCoordinatesMapper.hh>

#include <gmCore/MathConstants.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE(AngularFisheyeCoordinatesMapper);
GM_OFI_PARAM2(AngularFisheyeCoordinatesMapper, coverageAngle, gmCore::angle, setCoverageAngle);

struct AngularFisheyeCoordinatesMapper::Impl {
  struct uniforms {
    GLint ca = 0;
  };

  std::unordered_map<GLint, uniforms> loc;

  gmCore::angle coverage_angle = float(GM_PI);
};

AngularFisheyeCoordinatesMapper::AngularFisheyeCoordinatesMapper()
  : _impl(std::make_unique<Impl>()) {}
AngularFisheyeCoordinatesMapper::~AngularFisheyeCoordinatesMapper() {}

std::string AngularFisheyeCoordinatesMapper::getCommonCode() {
  return withVarId(R"lang=glsl(
uniform float ID_coverageAngle;

#ifndef PI2
#define PI2 1.57079632679489661923132169163975144209858469968755
#endif
)lang=glsl");
}

std::string AngularFisheyeCoordinatesMapper::getTo2DCode() {
  return withVarId(R"lang=glsl(
bool mapTo2D(vec3 pos3, out vec2 pos2) {

  float phi = atan(pos3.y, sqrt(pos3.x*pos3.x + pos3.z*pos3.z));
  float r = (PI2 - phi) / (0.5 * ID_coverageAngle);
  if (r > 1.0) return false;

  float theta = atan(pos3.x, pos3.z);

  pos2 = vec2(r * sin(theta), r * cos(theta));
  return true;
}
)lang=glsl");
}

std::string AngularFisheyeCoordinatesMapper::getTo3DCode() {
  return withVarId(R"lang=glsl(
bool mapTo3D(vec2 pos2, out vec3 pos3) {

  float r = sqrt(dot(pos2.xy, pos2.xy));
  if (r > 1) return false;

  float phi = 0.5 * r * ID_coverageAngle;
  float theta = atan(pos2.y, pos2.x);

  pos3 = vec3(cos(theta) * sin(phi), cos(phi), sin(theta) * sin(phi));
  return true;
}
)lang=glsl");
}

#define LOC(VAR, NAME)                                                         \
  (_impl->loc[program_id].VAR > 0                                              \
       ? _impl->loc[program_id].VAR                                            \
       : (_impl->loc[program_id].VAR =                                         \
              glGetUniformLocation(program_id, withVarId(NAME).c_str())))

void AngularFisheyeCoordinatesMapper::setCommonUniforms(GLuint program_id) {
  glUniform1f(LOC(ca, "ID_coverageAngle"), _impl->coverage_angle);
}

void AngularFisheyeCoordinatesMapper::setCoverageAngle(gmCore::angle a) {
  _impl->coverage_angle = a;
}

END_NAMESPACE_GMGRAPHICS;
