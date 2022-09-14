
#include <gmGraphics/EquirectangularCoordinatesMapper.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE(EquirectangularCoordinatesMapper);
GM_OFI_PARAM2(EquirectangularCoordinatesMapper, coverageAngle, gmCore::angle2, setCoverageAngle);

struct EquirectangularCoordinatesMapper::Impl {
  GLint h_loc = 0;
  GLint v_loc = 0;
  gmCore::angle2 coverage_angle = {float(GM_2_PI), float(GM_PI)};
};

EquirectangularCoordinatesMapper::EquirectangularCoordinatesMapper()
  : _impl(std::make_unique<Impl>()) {}
EquirectangularCoordinatesMapper::~EquirectangularCoordinatesMapper() {}

std::string EquirectangularCoordinatesMapper::getCommonCode() {
  return withVarId(R"lang=glsl(
uniform float ID_h_coverageAngle;
uniform float ID_v_coverageAngle;
)lang=glsl");
}

std::string EquirectangularCoordinatesMapper::getTo2DCode() {
  return withVarId(R"lang=glsl(
bool mapTo2D(vec3 pos3, out vec2 pos2) {

  float r = sqrt(dot(pos3.xz, pos3.xz));

  float phi = atan(pos3.y, r);
  float theta = atan(pos3.x, -pos3.z);

  pos2 = vec2(theta / ID_h_coverageAngle, phi / ID_v_coverageAngle);
  return true;
}
)lang=glsl");
}

std::string EquirectangularCoordinatesMapper::getTo3DCode() {
  return withVarId(R"lang=glsl(
bool mapTo3D(vec2 pos2, out vec3 pos3) {

  float ax = pos2.x * ID_h_coverageAngle;
  float ay = pos2.y * ID_v_coverageAngle;

  pos3 = vec3(cos(ay) * sin(ax), sin(ay), -cos(ay) * cos(ax));
  return true;
}
)lang=glsl");
}

#define LOC(VAR, NAME)                                                         \
  (VAR > 0                                                                     \
       ? VAR                                                                   \
       : (VAR = glGetUniformLocation(program_id, withVarId(NAME).c_str())))

void EquirectangularCoordinatesMapper::setCommonUniforms(GLuint program_id) {
  glUniform1f(LOC(_impl->h_loc, "ID_h_coverageAngle"),
              0.5f * _impl->coverage_angle[0]);
  glUniform1f(LOC(_impl->v_loc, "ID_v_coverageAngle"),
              0.5f * _impl->coverage_angle[1]);
}

void EquirectangularCoordinatesMapper::setCoverageAngle(gmCore::angle2 a) {
  _impl->coverage_angle = a;
}

END_NAMESPACE_GMGRAPHICS;
