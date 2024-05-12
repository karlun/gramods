
#include <gmGraphics/TransitionCoordinatesMapper.hh>

#include <gmCore/TimeTools.hh>
#include <gmCore/Updateable.hh>
#include <gmCore/PreConditionViolation.hh>


BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE(TransitionCoordinatesMapper);
GM_OFI_PARAM2(TransitionCoordinatesMapper, timeRange, gmCore::float2, setTimeRange);
GM_OFI_PARAM2(TransitionCoordinatesMapper, frameRange, gmCore::size2, setFrameRange);
GM_OFI_PARAM2(TransitionCoordinatesMapper, ratio, float, setRatio);
GM_OFI_POINTER2(TransitionCoordinatesMapper, coordinatesMapper, CoordinatesMapper, addCoordinatesMapper);

struct TransitionCoordinatesMapper::Impl : gmCore::Updateable {

  Impl() : start_time(gmCore::TimeTools::clock::now()) {}

  void update(clock::time_point time, size_t frame) override;
  std::string replace(std::string text, std::string A, std::string B);

  std::string getCommonCode();
  std::string getTo2DCode();
  std::string getTo3DCode();

  struct uniforms {
    GLint ratio = 0;
  };

  std::unordered_map<GLint, uniforms> loc;
  float ratio = 0.5f;

  std::optional<gmCore::float2> time_range;
  std::optional<gmCore::size2> frame_range;

  std::vector<std::shared_ptr<CoordinatesMapper>> mappers;

  gmCore::TimeTools::clock::time_point start_time;
};

TransitionCoordinatesMapper::TransitionCoordinatesMapper()
  : _impl(std::make_unique<Impl>()) {}
TransitionCoordinatesMapper::~TransitionCoordinatesMapper() {}

void TransitionCoordinatesMapper::Impl::update(clock::time_point time,
                                               size_t frame){
  if (time_range) {
    auto time_range = *this->time_range;
    double d = gmCore::TimeTools::durationToSeconds(time - start_time);
    if (d <= time_range[0]) ratio = 0.f;
    if (d >= time_range[1]) ratio = 1.f;
    ratio = (d - time_range[0]) / (time_range[1] - time_range[0]);
  }

  if (frame_range) {
    auto frame_range = *this->frame_range;
    if (frame <= frame_range[0]) ratio = 0.f;
    if (frame >= frame_range[1]) ratio = 1.f;
    ratio = (frame - frame_range[0]) / (frame_range[1] - frame_range[0]);
  }
}

std::string TransitionCoordinatesMapper::Impl::replace(std::string text,
                                                       std::string A,
                                                       std::string B) {
  size_t idx = 0;
  while ((idx = text.find(A, idx)) != std::string::npos) {
    text.replace(idx, A.size(), B);
    idx += B.size();
  }
  return text;
}

std::string TransitionCoordinatesMapper::getCommonCode() {
  return _impl->getCommonCode() + withVarId(R"lang=glsl(
uniform float ID_ratio;
)lang=glsl");
}

std::string TransitionCoordinatesMapper::Impl::getCommonCode() {
  if (mappers.size() < 2)
    throw gmCore::PreConditionViolation(GM_STR(
        "TransitionCoordinatesMapper requires two coordinates mappers; got "
        << mappers.size()));
  return mappers[0]->getCommonCode() + mappers[1]->getCommonCode();
}

std::string TransitionCoordinatesMapper::getTo2DCode() {
  return _impl->getTo2DCode() + withVarId(R"lang=glsl(
bool mapTo2D(vec3 pos3, out vec2 pos2) {

  vec2 pos2_1;
  bool good_1 = mapTo2D_1(pos3, pos2_1);

  vec2 pos2_2;
  bool good_2 = mapTo2D_2(pos3, pos2_2);

  if (!good_1 || !good_2) return false;

  pos2 = (1 - ID_ratio) * pos2_1 + ID_ratio * pos2_2;

  return true;
}
)lang=glsl");
}

std::string TransitionCoordinatesMapper::Impl::getTo2DCode() {
  if (mappers.size() < 2)
    throw gmCore::PreConditionViolation(GM_STR(
        "TransitionCoordinatesMapper requires two coordinates mappers; got "
        << mappers.size()));
  return replace(mappers[0]->getTo2DCode(), "mapTo2D", "mapTo2D_1") +
         replace(mappers[1]->getTo2DCode(), "mapTo2D", "mapTo2D_2");
}

std::string TransitionCoordinatesMapper::getTo3DCode() {
  return _impl->getTo3DCode() + withVarId(R"lang=glsl(
vec3 TCM_SLERP(vec3 A, vec3 B, float r) {
  float dotp = dot(A, B);
  if ((dotp > 0.9) || (dotp < -0.9))
    return normalize((1 - r) * A + r * B);
  float theta = acos(dotp);
  return (A * sin((1 - r) * theta) + B * sin(r * theta)) / sin(theta);
}

bool mapTo3D(vec2 pos2, out vec3 pos3) {

  vec3 pos3_1;
  bool good_1 = mapTo3D_1(pos2, pos3_1);

  vec3 pos3_2;
  bool good_2 = mapTo3D_2(pos2, pos3_2);

  if (!good_1 || !good_2) return false;

  pos3 = TCM_SLERP(pos3_1, pos3_2, ID_ratio);

  return true;
}
)lang=glsl");
}

std::string TransitionCoordinatesMapper::Impl::getTo3DCode() {
  if (mappers.size() < 2)
    throw gmCore::PreConditionViolation(GM_STR(
        "TransitionCoordinatesMapper requires two coordinates mappers; got "
        << mappers.size()));
  return replace(mappers[0]->getTo3DCode(), "mapTo3D", "mapTo3D_1") +
         replace(mappers[1]->getTo3DCode(), "mapTo3D", "mapTo3D_2");
}

#define LOC(VAR, NAME)                                                         \
  (_impl->loc[program_id].VAR > 0                                              \
       ? _impl->loc[program_id].VAR                                            \
       : (_impl->loc[program_id].VAR =                                         \
              glGetUniformLocation(program_id, withVarId(NAME).c_str())))

void TransitionCoordinatesMapper::setCommonUniforms(GLuint program_id) {
  for (auto &mapper : _impl->mappers) mapper->setCommonUniforms(program_id);
  glUniform1f(LOC(ratio, "ID_ratio"), _impl->ratio);
}

void TransitionCoordinatesMapper::setTimeRange(gmCore::float2 t) {
  if (t[0] > t[1])
    throw gmCore::InvalidArgument(
        GM_STR("start (" << t[0] << ") is later than end (" << t[1] << ")"));
  _impl->frame_range = std::nullopt;
  _impl->time_range = t;
}

void TransitionCoordinatesMapper::setFrameRange(gmCore::size2 f) {
  if (f[0] > f[1])
    throw gmCore::InvalidArgument(
        GM_STR("start (" << f[0] << ") is later than end (" << f[1] << ")"));
  _impl->time_range = std::nullopt;
  _impl->frame_range = f;
}

void TransitionCoordinatesMapper::setRatio(float r) {
  _impl->ratio = r;
}

void TransitionCoordinatesMapper::addCoordinatesMapper(
    std::shared_ptr<CoordinatesMapper> m) {
  if (!m) throw gmCore::InvalidArgument("null not allowed");
  _impl->mappers.push_back(m);
}

void TransitionCoordinatesMapper::traverse(Visitor *visitor) {
  for (auto &m : _impl->mappers) m->accept(visitor);
}

END_NAMESPACE_GMGRAPHICS;
