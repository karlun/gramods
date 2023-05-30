
#include <gmGraphics/RectilinearCameraModel.hh>

#include <gmCore/RunOnce.hh>


BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE(RectilinearCameraModel);
GM_OFI_PARAM2(RectilinearCameraModel, kDistortion, gmCore::float3, setKDistortion);
GM_OFI_PARAM2(RectilinearCameraModel, pDistortion, gmCore::float2, setPDistortion);
GM_OFI_PARAM2(RectilinearCameraModel, focalDistance, gmCore::float2, setFocalDistance);
GM_OFI_PARAM2(RectilinearCameraModel, focalOffset, gmCore::float2, setFocalOffset);

struct RectilinearCameraModel::Impl {

  gmCore::float3 k_dist = { 0.f, 0.f, 0.f };
  gmCore::float2 p_dist = { 0.f, 0.f };
  gmCore::float2 focal = { 1.f, 1.f };
  gmCore::float2 offset = {0.5f, 0.5f };

  struct uniforms {
    GLint k = 0;
    GLint p = 0;

    GLint fx = 0;
    GLint fy = 0;
    GLint cx = 0;
    GLint cy = 0;
  };

  std::unordered_map<GLint, uniforms> loc;
};

RectilinearCameraModel::RectilinearCameraModel()
  : _impl(std::make_unique<Impl>()) {}
RectilinearCameraModel::~RectilinearCameraModel() {}

std::string RectilinearCameraModel::getCommonCode() {
  return withVarId(R"lang=glsl(
uniform float ID_fx;
uniform float ID_fy;
uniform float ID_cx;
uniform float ID_cy;
)lang=glsl");
}

std::string RectilinearCameraModel::getTo3DCode() {
  GM_RUNONCE_BEGIN;
  if (_impl->k_dist[0] != 0.f || _impl->k_dist[1] != 0.f ||
      _impl->k_dist[2] != 0.f || _impl->p_dist[0] != 0.f ||
      _impl->p_dist[1] != 0.f)
    GM_WRN("RectilinearCameraModel",
           "Reverse mapping does not support distortion");
  GM_RUNONCE_END;

  return withVarId(R"lang=glsl(
bool mapTo3D(vec2 pos2, out vec3 pos3) {

  vec2 uv = 0.5 * (pos2 + 1);
  float x = (uv.x - ID_cx) / ID_fx;
  float y = (uv.y - ID_cy) / ID_fy;

  // Go from OpenCV to OpenGL coordinates
  pos3 = normalize(vec3(x, y, -1));

  return true;
}
)lang=glsl");
}

std::string RectilinearCameraModel::getTo2DCode() {
  return withVarId(R"lang=glsl(
uniform vec3 ID_k;
uniform vec2 ID_p;

bool mapTo2D(vec3 pos3, out vec2 pos2) {
  vec3 k = ID_k;
  vec2 p = ID_p;

  float fx = ID_fx;
  float fy = ID_fy;
  float cx = ID_cx;
  float cy = ID_cy;

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
)lang=glsl");
}

#define LOC(VAR, NAME)                                                         \
  (_impl->loc[program_id].VAR > 0                                              \
       ? _impl->loc[program_id].VAR                                            \
       : (_impl->loc[program_id].VAR =                                         \
              glGetUniformLocation(program_id, withVarId(NAME).c_str())))

void RectilinearCameraModel::setCommonUniforms(GLuint program_id) {
  glUniform1f(LOC(fx, "ID_fx"), _impl->focal[0]);
  glUniform1f(LOC(fy, "ID_fy"), _impl->focal[1]);
  glUniform1f(LOC(cx, "ID_cx"), _impl->offset[0]);
  glUniform1f(LOC(cy, "ID_cy"), _impl->offset[1]);
}

void RectilinearCameraModel::setTo2DUniforms(GLuint program_id) {
  glUniform3f(
      LOC(k, "ID_k"), _impl->k_dist[0], _impl->k_dist[1], _impl->k_dist[2]);
  glUniform2f(LOC(p, "ID_p"), _impl->p_dist[0], _impl->p_dist[1]);
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
