
#include <gmGraphics/FisheyeCameraModel.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE(FisheyeCameraModel);
GM_OFI_PARAM2(FisheyeCameraModel, distortion, gmCore::float4, setDistortion);
GM_OFI_PARAM2(FisheyeCameraModel, focalDistance, gmCore::float2, setFocalDistance);
GM_OFI_PARAM2(FisheyeCameraModel, focalOffset, gmCore::float2, setFocalOffset);

struct FisheyeCameraModel::Impl {

  gmCore::float4 k_dist = { 0.f, 0.f, 0.f, 0.f };
  gmCore::float2 focal = { 1.f, 1.f };
  gmCore::float2 offset = { 0.5f, 0.5f };

  struct uniforms {
    GLint k = 0;

    GLint fx = 0;
    GLint fy = 0;
    GLint cx = 0;
    GLint cy = 0;
  };

  std::unordered_map<GLint, uniforms> loc;
};

FisheyeCameraModel::FisheyeCameraModel()
  : _impl(std::make_unique<Impl>()) {}
FisheyeCameraModel::~FisheyeCameraModel() {}

std::string FisheyeCameraModel::getTo2DCode() {
  return withVarId(R"lang=glsl(

uniform vec4 ID_k;

uniform float ID_fx;
uniform float ID_fy;
uniform float ID_cx;
uniform float ID_cy;

bool mapTo2D(vec3 pos3, out vec2 pos2) {

  // Go to OpenCV coordinates
  float x = pos3.x;
  float y = pos3.y;
  float z = -pos3.z;

  if (z <= 0) return false;

  float a = x / z;
  float b = y / z;
  
  float r = sqrt(a*a + b*b);
  if (r < 1e-10) r = 1;
  
  float th = atan(r);
  float thd = th * (1 + k.x * pow(th,2) + k.y * pow(th,4) + k.z * pow(th,6) + k.w * pow(th,8));
  
  float xp = (thd/r)*a;
  float yp = (thd/r)*b;

  float u = fx * xp + cx;
  float v = fy * yp + cy;

  if (u < 0 || 1 < u ||
      v < 0 || 1 < v)
    return false;

  pos2 = 2 * vec2(u, v) - 1;

  return true;
}
)lang=glsl");
}

#define LOC(VAR, NAME)                                                         \
  (_impl->loc[program_id].VAR > 0                                              \
       ? _impl->loc[program_id].VAR                                            \
       : (_impl->loc[program_id].VAR =                                         \
              glGetUniformLocation(program_id, withVarId(NAME).c_str())))

void FisheyeCameraModel::setTo2DUniforms(GLuint program_id) {
  glUniform4fv(LOC(k, "ID_k"), 1, _impl->k_dist.data());
  glUniform1f(LOC(fx, "ID_fx"), _impl->focal[0]);
  glUniform1f(LOC(fy, "ID_fy"), _impl->focal[1]);
  glUniform1f(LOC(cx, "ID_cx"), _impl->offset[0]);
  glUniform1f(LOC(cy, "ID_cy"), _impl->offset[1]);
}

void FisheyeCameraModel::setDistortion(gmCore::float4 k) {
  _impl->k_dist = k;
}

void FisheyeCameraModel::setFocalDistance(gmCore::float2 f) {
  _impl->focal = f;
}

void FisheyeCameraModel::setFocalOffset(gmCore::float2 c) {
  _impl->offset = c;
}

END_NAMESPACE_GMGRAPHICS;
