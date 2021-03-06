
#include <gmGraphics/RectilinearCameraModel.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE(RectilinearCameraModel);
GM_OFI_PARAM2(RectilinearCameraModel, kDistortion, gmCore::float3, setKDistortion);
GM_OFI_PARAM2(RectilinearCameraModel, pDistortion, gmCore::float2, setPDistortion);
GM_OFI_PARAM2(RectilinearCameraModel, focalDistance, gmCore::float2, setFocalDistance);
GM_OFI_PARAM2(RectilinearCameraModel, focalOffset, gmCore::float2, setFocalOffset);

struct RectilinearCameraModel::Impl {

  void setMapperUniforms(GLuint program_id);

  gmCore::float3 k_dist = { 0.f, 0.f, 0.f };
  gmCore::float2 p_dist = { 0.f, 0.f };
  gmCore::float2 focal = { 1.f, 1.f };
  gmCore::float2 offset = {0.5f, 0.5f };
};

RectilinearCameraModel::RectilinearCameraModel()
  : _impl(std::make_unique<Impl>()) {}
RectilinearCameraModel::~RectilinearCameraModel() {}

std::string RectilinearCameraModel::getMapperCode() {
  static const std::string code = R"lang=glsl(

uniform vec3 k;
uniform vec2 p;

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

  float x2 = x / z;
  float y2 = y / z;
  
  float r = sqrt(x2*x2 + y2*y2);
  
  float x3 = x2 * (1 + k.x * pow(r,2) + k.y * pow(r,4) + k.z * pow(r,6)) + 2*p.x*x2*y2 + p.y*(r*r + 2*x2*x2);
  float y3 = y2 * (1 + k.x * pow(r,2) + k.y * pow(r,4) + k.z * pow(r,6)) + p.x*(r*r + 2*y2*y2) + 2*p.y*x2*y2;

  float u = fx * x3 + cx;
  float v = fy * y3 + cy;

  if (u < 0 || 1 < u ||
      v < 0 || 1 < v)
    return false;

  pos2 = 2 * vec2(u, v) - 1;

  return true;
}
)lang=glsl";
  return code;
}

void RectilinearCameraModel::setMapperUniforms(GLuint program_id) {
  _impl->setMapperUniforms(program_id);
}

void RectilinearCameraModel::Impl::setMapperUniforms(GLuint program_id) {
  glUniform3f(glGetUniformLocation(program_id, "k"), k_dist[0], k_dist[1], k_dist[2]);
  glUniform2f(glGetUniformLocation(program_id, "p"), p_dist[0], p_dist[1]);
  glUniform1f(glGetUniformLocation(program_id, "fx"), focal[0]);
  glUniform1f(glGetUniformLocation(program_id, "fy"), focal[1]);
  glUniform1f(glGetUniformLocation(program_id, "cx"), offset[0]);
  glUniform1f(glGetUniformLocation(program_id, "cy"), offset[1]);
}

void RectilinearCameraModel::setKDistortion(gmCore::float3 k) {
  _impl->k_dist = k;
}

void RectilinearCameraModel::setPDistortion(gmCore::float2 p) {
  _impl->p_dist = p;
}

void RectilinearCameraModel::setFocalDistance(gmCore::float2 f) {
  _impl->focal = f;
}

void RectilinearCameraModel::setFocalOffset(gmCore::float2 c) {
  _impl->offset = c;
}

END_NAMESPACE_GMGRAPHICS;
