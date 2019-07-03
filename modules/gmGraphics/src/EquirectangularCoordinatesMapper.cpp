
#include <gmGraphics/EquirectangularCoordinatesMapper.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE(EquirectangularCoordinatesMapper);

EquirectangularCoordinatesMapper::~EquirectangularCoordinatesMapper() {}

std::string EquirectangularCoordinatesMapper::getMapperCode() {
  static const std::string code = R"lang=glsl(

#define PI1 3.14159265358979323846264338327950288419716939937511
#define PI2 1.57079632679489661923132169163975144209858469968755

bool mapTo2D(vec3 pos3, out vec2 pos2) {

  float r = sqrt(dot(pos3.xz, pos3.xz));

  float phi = atan(pos3.y, r);
  float theta = atan(pos3.x, -pos3.z);

  pos2 = vec2(theta / PI1, phi / PI2);
  return true;
}

bool mapTo3D(vec2 pos2, out vec3 pos3) {

  float ay = pos2.y * PI2;
  float ax = pos2.x * PI1;

  pos3 = vec3(cos(ay) * sin(ax), sin(ay), -cos(ay) * cos(ax));
  return true;
}
)lang=glsl";
  return code;
}

END_NAMESPACE_GMGRAPHICS;
