
#include <gmGraphics/EquirectangularCoordinatesMapper.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE(EquirectangularCoordinatesMapper);
GM_OFI_PARAM2(EquirectangularCoordinatesMapper, coverageAngle, gmCore::angle2, setCoverageAngle);

struct EquirectangularCoordinatesMapper::Impl {
  gmCore::angle2 coverage_angle = {float(GM_2_PI), float(GM_PI)};
};

EquirectangularCoordinatesMapper::EquirectangularCoordinatesMapper()
  : _impl(std::make_unique<Impl>()) {}
EquirectangularCoordinatesMapper::~EquirectangularCoordinatesMapper() {}

std::string EquirectangularCoordinatesMapper::getMapperCode() {
  static const std::string code = R"lang=glsl(

uniform float h_coverageAngle;
uniform float v_coverageAngle;

bool mapTo2D(vec3 pos3, out vec2 pos2) {

  float r = sqrt(dot(pos3.xz, pos3.xz));

  float phi = atan(pos3.y, r);
  float theta = atan(pos3.x, -pos3.z);

  pos2 = vec2(theta / h_coverageAngle, phi / v_coverageAngle);
  return true;
}

bool mapTo3D(vec2 pos2, out vec3 pos3) {

  float ax = pos2.x * h_coverageAngle;
  float ay = pos2.y * v_coverageAngle;

  pos3 = vec3(cos(ay) * sin(ax), sin(ay), -cos(ay) * cos(ax));
  return true;
}
)lang=glsl";
  return code;
}

void EquirectangularCoordinatesMapper::setMapperUniforms(GLuint program_id) {
  glUniform1f(glGetUniformLocation(program_id, "h_coverageAngle"),
              0.5f * _impl->coverage_angle[0]);
  glUniform1f(glGetUniformLocation(program_id, "v_coverageAngle"),
              0.5f * _impl->coverage_angle[1]);
}

void EquirectangularCoordinatesMapper::setCoverageAngle(gmCore::angle2 a) {
  _impl->coverage_angle = a;
}

END_NAMESPACE_GMGRAPHICS;
