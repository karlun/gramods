
#include <gmGraphics/MixingShaders.hh>

#include <gmCore/Console.hh>
#include <gmCore/RunOnce.hh>

#include <unordered_map>
#include <string>
#include <sstream>

namespace {
struct MixData {
  const size_t min_tex_count;
  const size_t max_tex_count;
  const std::string fragment_code;
};

const std::unordered_map<std::string, MixData> mix_type_data = {
    {"average",
     MixData {.min_tex_count = 1,
              .max_tex_count = 8,
              .fragment_code =
                  R"lang=glsl(
#version 330 core

uniform sampler2D tex[];
uniform int tex_count;

in vec2 position;

out vec4 fragColor;

void main() {

  float mix = 1.0 / tex_count;

  vec3 rgb = mix * texture(tex[0], position * 0.5 + 0.5).rgb;
  if (tex_count > 1)
    rgb += mix * texture(tex[1], position * 0.5 + 0.5).rgb;
  if (tex_count > 2)
    rgb += mix * texture(tex[2], position * 0.5 + 0.5).rgb;
  if (tex_count > 3)
    rgb += mix * texture(tex[3], position * 0.5 + 0.5).rgb;
  if (tex_count > 4)
    rgb += mix * texture(tex[4], position * 0.5 + 0.5).rgb;
  if (tex_count > 5)
    rgb += mix * texture(tex[5], position * 0.5 + 0.5).rgb;
  if (tex_count > 6)
    rgb += mix * texture(tex[6], position * 0.5 + 0.5).rgb;
  if (tex_count > 7)
    rgb += mix * texture(tex[7], position * 0.5 + 0.5).rgb;

  fragColor = vec4(rgb, 1);
}
)lang=glsl"}},

    {"difference",
     MixData {.min_tex_count = 2,
              .max_tex_count = 2,
              .fragment_code =
                  R"lang=glsl(
#version 330 core

uniform sampler2D tex[];
uniform int tex_count;

in vec2 position;

out vec4 fragColor;

void main() {
  vec3 rgb = texture(tex[0], position * 0.5 + 0.5).rgb - texture(tex[1], position * 0.5 + 0.5).rgb;
  fragColor = vec4(abs(rgb), 1);
}
)lang=glsl"}},

    {"distance",
     MixData {.min_tex_count = 2,
              .max_tex_count = 2,
              .fragment_code =
                  R"lang=glsl(
#version 330 core

uniform sampler2D tex[];
uniform int tex_count;

in vec2 position;

out vec4 fragColor;

void main() {
  vec3 rgb0 = texture(tex[0], position * 0.5 + 0.5).rgb;
  vec3 rgb1 = texture(tex[1], position * 0.5 + 0.5).rgb;
  float dist = length(rgb1 - rgb0);
  fragColor = vec4(dist, dist, dist, 1);
}
)lang=glsl"}},

    {"alpha mask",
     MixData {.min_tex_count = 2,
              .max_tex_count = 2,
              .fragment_code =
                  R"lang=glsl(
#version 330 core

uniform sampler2D tex[];
uniform int tex_count;

in vec2 position;

out vec4 fragColor;

void main() {
  float A0 = texture(tex[0], position * 0.5 + 0.5).a;
  vec3 rgb1 = texture(tex[1], position * 0.5 + 0.5).rgb;
  fragColor = vec4(rgb1, A0);
}
)lang=glsl"}},

    {"red mask",
     MixData {.min_tex_count = 2,
              .max_tex_count = 2,
              .fragment_code =
                  R"lang=glsl(
#version 330 core

uniform sampler2D tex[];
uniform int tex_count;

in vec2 position;

out vec4 fragColor;

void main() {
  float A0 = texture(tex[0], position * 0.5 + 0.5).r;
  vec3 rgb1 = texture(tex[1], position * 0.5 + 0.5).rgb;
  fragColor = vec4(rgb1, A0);
}
)lang=glsl"}},

    {"alpha weighted average",
     MixData {.min_tex_count = 3,
              .max_tex_count = 3,
              .fragment_code =
                  R"lang=glsl(
#version 330 core

uniform sampler2D tex[];
uniform int tex_count;

in vec2 position;

out vec4 fragColor;

void main() {
  float A0 = texture(tex[0], position * 0.5 + 0.5).a;

  vec3 rgb = ( A0 ) * texture(tex[1], position * 0.5 + 0.5).rgb +
             (1-A0) * texture(tex[2], position * 0.5 + 0.5).rgb;
  fragColor = vec4(rgb, 1);
}
)lang=glsl"}},

    {"red weighted average",
     MixData {.min_tex_count = 3,
              .max_tex_count = 3,
              .fragment_code =
                  R"lang=glsl(
#version 330 core

uniform sampler2D tex[];
uniform int tex_count;

in vec2 position;

out vec4 fragColor;

void main() {
  float A0 = texture(tex[0], position * 0.5 + 0.5).r;

  vec3 rgb = ( A0 ) * texture(tex[1], position * 0.5 + 0.5).rgb +
             (1-A0) * texture(tex[2], position * 0.5 + 0.5).rgb;
  fragColor = vec4(rgb, 1);
}
)lang=glsl"}}};
}

BEGIN_NAMESPACE_GMGRAPHICS;

bool MixingShaders::checkMixTypeValid(const std::string &type,
                                      const std::string &name) {
  if (mix_type_data.count(type) > 0) return true;
  std::stringstream ss;
  for (const auto &item : mix_type_data) {
    if (!ss.str().empty()) [[likely]]
      ss << ", ";
    ss << item.first;
  }
  GM_WRN(name,
         "Invalid mix type '" << type
                              << "'; valid alternatives are: " << ss.str());
  return false;
}

bool MixingShaders::checkCount(const std::string &mix_type,
                               size_t count,
                               const std::string &name,
                               const std::string &type) {

  if (!checkMixTypeValid(mix_type, name)) return false;

  const auto &data = mix_type_data.at(mix_type);
  if (count < data.min_tex_count) {
    GM_RUNONCE(GM_ERR(
        name,
        "Too few " << type << " to mix for the current operator ('" << mix_type
                   << "', " << count << " < " << data.min_tex_count << ")."));
    return false;
  }

  if (count > data.max_tex_count) {
    GM_RUNONCE(GM_WRN(
        name,
        "Too many " << type << " to mix for the current operator ('" << mix_type
                    << "', " << count << " > " << data.max_tex_count
                    << ") - additional will be ignored."));
  }

  return true;
}

std::string MixingShaders::getFragmentCode(const std::string &type) {
  return mix_type_data.at(type).fragment_code;
}

END_NAMESPACE_GMGRAPHICS;
