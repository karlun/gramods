
#include <gmGraphics/FisheyeCameraModel.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE(FisheyeCameraModel);
GM_OFI_PARAM(FisheyeCameraModel, distortion, gmCore::float4, FisheyeCameraModel::setDistortion);
GM_OFI_PARAM(FisheyeCameraModel, focalDistance, gmCore::float2, FisheyeCameraModel::setFocalDistance);
GM_OFI_PARAM(FisheyeCameraModel, focalOffset, gmCore::float2, FisheyeCameraModel::setFocalOffset);

struct FisheyeCameraModel::Impl {

  void setMapperUniforms(GLuint program_id);

  gmCore::float4 k_dist = { 0.f, 0.f, 0.f, 0.f };
  gmCore::float2 focal = { 1.f, 1.f };
  gmCore::float2 offset = { 0.5f, 0.5f };
};

FisheyeCameraModel::FisheyeCameraModel()
  : _impl(std::make_unique<Impl>()) {}
FisheyeCameraModel::~FisheyeCameraModel() {}

std::string FisheyeCameraModel::getMapperCode() {
  static const std::string code = R"lang=glsl(

uniform vec4 k;

uniform float fx;
uniform float fy;
uniform float cx;
uniform float cy;

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
)lang=glsl";
  return code;
}

void FisheyeCameraModel::setMapperUniforms(GLuint program_id) {
  _impl->setMapperUniforms(program_id);
}

void FisheyeCameraModel::Impl::setMapperUniforms(GLuint program_id) {
  glUniform4f(glGetUniformLocation(program_id, "k"), k_dist[0], k_dist[1], k_dist[2], k_dist[3]);
  glUniform1f(glGetUniformLocation(program_id, "fx"), focal[0]);
  glUniform1f(glGetUniformLocation(program_id, "fy"), focal[1]);
  glUniform1f(glGetUniformLocation(program_id, "cx"), offset[0]);
  glUniform1f(glGetUniformLocation(program_id, "cy"), offset[1]);
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
