
#include <gmGraphics/AngularFisheyeCoordinatesMapper.hh>

#include <gmCore/MathConstants.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE(AngularFisheyeCoordinatesMapper);
GM_OFI_PARAM2(AngularFisheyeCoordinatesMapper, coverageAngle, gmCore::angle, setCoverageAngle);

struct AngularFisheyeCoordinatesMapper::Impl {
  gmCore::angle coverage_angle = GM_2_PI;
};

AngularFisheyeCoordinatesMapper::AngularFisheyeCoordinatesMapper()
  : _impl(std::make_unique<Impl>()) {}
AngularFisheyeCoordinatesMapper::~AngularFisheyeCoordinatesMapper() {}

std::string AngularFisheyeCoordinatesMapper::getMapperCode() {
  static const std::string code = R"lang=glsl(

uniform float coverageAngle;

#define PI1 3.14159265358979323846264338327950288419716939937511
#define PI2 1.57079632679489661923132169163975144209858469968755

bool mapTo2D(vec3 pos3, out vec2 pos2) {

  float phi = atan(pos3.y, sqrt(pos3.x*pos3.x + pos3.z*pos3.z));
  float r = (PI2 - phi) / (0.5 * coverageAngle);
  if (r > 1.0) return false;

  float theta = atan(pos3.x, pos3.z);

  pos2 = vec2(r * sin(theta), r * cos(theta));
  return true;
}

bool mapTo3D(vec2 pos2, out vec3 pos3) {

  float r = sqrt(dot(pos2.xy, pos2.xy));
  if (r > 1) return false;

  float phi = 0.5 * r * coverageAngle;
  float theta = atan(pos2.y, pos2.x);

  pos3 = vec3(cos(theta) * sin(phi), cos(phi), sin(theta) * sin(phi));
  return true;
}
)lang=glsl";
  return code;
}

void AngularFisheyeCoordinatesMapper::setMapperUniforms(GLuint program_id) {
  glUniform1f(glGetUniformLocation(program_id, "coverageAngle"), _impl->coverage_angle);
}

void AngularFisheyeCoordinatesMapper::setCoverageAngle(gmCore::angle a) {
  _impl->coverage_angle = a;
}

END_NAMESPACE_GMGRAPHICS;
