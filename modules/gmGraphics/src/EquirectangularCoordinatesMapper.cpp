
#include <gmGraphics/EquirectangularCoordinatesMapper.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE(EquirectangularCoordinatesMapper);
GM_OFI_PARAM2(EquirectangularCoordinatesMapper, coverageAngle, gmCore::angle2, setCoverageAngle);
GM_OFI_PARAM2(EquirectangularCoordinatesMapper, coverageRange, gmCore::angle4, setCoverageRange);

struct EquirectangularCoordinatesMapper::Impl {

  struct uniforms {
    GLint h0 = 0, h1 = 0, v0 = 0, v1 = 0;
  };

  std::unordered_map<GLint, uniforms> loc;

  gmCore::angle4 coverage_range = {
      float(-GM_PI), float(GM_PI), float(-GM_PI_2), float(GM_PI_2)};
};

EquirectangularCoordinatesMapper::EquirectangularCoordinatesMapper()
  : _impl(std::make_unique<Impl>()) {}
EquirectangularCoordinatesMapper::~EquirectangularCoordinatesMapper() {}

std::string EquirectangularCoordinatesMapper::getCommonCode() {
  return withVarId(R"lang=glsl(
uniform float ID_h_angle0;
uniform float ID_h_angle1;
uniform float ID_v_angle0;
uniform float ID_v_angle1;
)lang=glsl");
}

std::string EquirectangularCoordinatesMapper::getTo2DCode() {
  return withVarId(R"lang=glsl(
bool mapTo2D(vec3 pos3, out vec2 pos2) {

  float r = sqrt(dot(pos3.xz, pos3.xz));

  float phi = atan(pos3.y, r);
  float theta = atan(pos3.x, -pos3.z);

  pos2 = vec2(2.0 * (theta - ID_h_angle0) / (ID_h_angle1 - ID_h_angle0) - 1.0,
              2.0 * (phi   - ID_v_angle0) / (ID_v_angle1 - ID_v_angle0) - 1.0);
  return true;
}
)lang=glsl");
}

std::string EquirectangularCoordinatesMapper::getTo3DCode() {
  return withVarId(R"lang=glsl(
bool mapTo3D(vec2 pos2, out vec3 pos3) {

  float ax = 0.5 * (pos2.x + 1.0) * (ID_h_angle1 - ID_h_angle0) + ID_h_angle0;
  //float ax = pos2.x * (ID_h_angle1 - ID_h_angle0) + ID_h_angle0 + 1.57079632679489661923;
  float ay = 0.5 * (pos2.y + 1.0) * (ID_v_angle1 - ID_v_angle0) + ID_v_angle0;
  //float ay = pos2.y * (ID_v_angle1 - ID_v_angle0) + ID_v_angle0 + 0.78539816339744830962;

  pos3 = vec3(cos(ay) * sin(ax), sin(ay), -cos(ay) * cos(ax));
  return true;
}
)lang=glsl");
}

#define LOC(VAR, NAME)                                                         \
  (_impl->loc[program_id].VAR > 0                                              \
       ? _impl->loc[program_id].VAR                                            \
       : (_impl->loc[program_id].VAR =                                         \
              glGetUniformLocation(program_id, withVarId(NAME).c_str())))

void EquirectangularCoordinatesMapper::setCommonUniforms(GLuint program_id) {
  glUniform1f(LOC(h0, "ID_h_angle0"), _impl->coverage_range[0]);
  glUniform1f(LOC(h1, "ID_h_angle1"), _impl->coverage_range[1]);
  glUniform1f(LOC(v0, "ID_v_angle0"), _impl->coverage_range[2]);
  glUniform1f(LOC(v1, "ID_v_angle1"), _impl->coverage_range[3]);
}

void EquirectangularCoordinatesMapper::setCoverageAngle(gmCore::angle2 a) {
  _impl->coverage_range = {
      -0.5f * a[0], 0.5f * a[0], -0.5f * a[1], 0.5f * a[1]};
}

void EquirectangularCoordinatesMapper::setCoverageRange(gmCore::angle4 a) {
  _impl->coverage_range = a;
}

END_NAMESPACE_GMGRAPHICS;
