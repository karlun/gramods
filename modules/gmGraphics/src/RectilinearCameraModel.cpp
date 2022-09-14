
#include <gmGraphics/RectilinearCameraModel.hh>

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

  GLint k_loc = 0;
  GLint p_loc = 0;

  GLint fx_loc = 0;
  GLint fy_loc = 0;
  GLint cx_loc = 0;
  GLint cy_loc = 0;
};

RectilinearCameraModel::RectilinearCameraModel()
  : _impl(std::make_unique<Impl>()) {}
RectilinearCameraModel::~RectilinearCameraModel() {}

std::string RectilinearCameraModel::getTo2DCode() {
  return R"lang=glsl(
uniform vec3 ID_k;
uniform vec2 ID_p;

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
}

#define LOC(VAR, NAME)                                                         \
  (VAR > 0                                                                     \
       ? VAR                                                                   \
       : (VAR = glGetUniformLocation(program_id, withVarId(NAME).c_str())))

void RectilinearCameraModel::setTo2DUniforms(GLuint program_id) {
  glUniform3f(LOC(_impl->k_loc, "ID_k"),
              _impl->k_dist[0],
              _impl->k_dist[1],
              _impl->k_dist[2]);
  glUniform2f(LOC(_impl->p_loc, "ID_p"), _impl->p_dist[0], _impl->p_dist[1]);
  glUniform1f(LOC(_impl->fx_loc, "ID_fx"), _impl->focal[0]);
  glUniform1f(LOC(_impl->fy_loc, "ID_fy"), _impl->focal[1]);
  glUniform1f(LOC(_impl->cx_loc, "ID_cx"), _impl->offset[0]);
  glUniform1f(LOC(_impl->cy_loc, "ID_cy"), _impl->offset[1]);
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
