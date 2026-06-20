

#include <gmGraphics/ChromaKeyTexture.hh>

#include <gmGraphics/OffscreenRenderTargets.hh>
#include <gmGraphics/RasterProcessor.hh>

#include <gmCore/RunOnce.hh>
#include <gmCore/Console.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE(ChromaKeyTexture);
GM_OFI_POINTER2(ChromaKeyTexture, texture, TextureInterface, setTexture);
GM_OFI_PARAM2(ChromaKeyTexture, key, gmCore::float3, setKey);
GM_OFI_PARAM2(ChromaKeyTexture, tolerance, gmCore::float2, setTolerance);

struct ChromaKeyTexture::Impl {

  std::optional<TextureInterface::TextureData>
  updateTexture(size_t frame_number, Eye e);

  static const std::string fragment_code;

  OffscreenRenderTargets render_target;
  RasterProcessor raster_processor;
  GLuint texture_id = 0;
  size_t texture_frame = std::numeric_limits<size_t>::max();
  bool is_setup = false;
  bool is_functional = false;

  std::shared_ptr<TextureInterface> texture;
  gmCore::float3 key = { 0.f, 1.f, 0.f };
  gmCore::float2 tolerance = { 0.48f, 0.5f };
};

const std::string ChromaKeyTexture::Impl::fragment_code =
  R"lang=glsl(
#version 330 core

uniform sampler2D tex;
uniform vec3 key_rgb;
uniform float r0;
uniform float r1;

in vec2 position;

out vec4 fragColor;

vec3 rgb2ycbcr (vec3 rgb) {
  return vec3(0.299 * rgb.r + 0.587 * rgb.g + 0.114 * rgb.b,
              0.5 - 0.168736 * rgb.r - 0.331264 * rgb.g + 0.5 * rgb.b,
              0.5 + 0.5 * rgb.r - 0.418688 * rgb.g - 0.081312 * rgb.b);
}

float maskvalue(vec3 frag, float r0, float r1) {

  vec3 key_cc = rgb2ycbcr(key_rgb);
  vec3 frag_cc = rgb2ycbcr(frag);

  vec2 CbCr_diff = key_cc.yz - frag_cc.yz;
  float CbCr_dist = length(CbCr_diff);

  if (CbCr_dist < r0) return 0.0;
  if (CbCr_dist > r1) return 1.0;

  return (CbCr_dist - r0) / (r1 - r0);
}

vec4 chroma_key(vec4 color) {
  float mask = maskvalue(color.rgb, r0, r1);
  return vec4(color.rgb - (1 - mask) * key_rgb, mask * color.a);
}

void main() {
  fragColor = chroma_key(texture(tex, position * 0.5 + 0.5));
}
)lang=glsl";

ChromaKeyTexture::ChromaKeyTexture()
  : _impl(std::make_unique<Impl>()) {}

std::optional<TextureInterface::TextureData>
ChromaKeyTexture::Impl::updateTexture(size_t frame_number, Eye eye) {

  if (!texture) {
    GM_RUNONCE(GM_ERR("ChromaKeyTexture", "No texture to process."));
    return std::nullopt;
  }

  if (!is_setup) {
    is_setup = true;
    raster_processor.setFragmentCode(fragment_code);
    render_target.setLinearInterpolation(false);
    if (render_target.init(1) && raster_processor.init()) {
      is_functional = true;
      texture_id = render_target.getTexId(0);
    }
  }

  if (!is_functional) {
    GM_RUNONCE(GM_ERR("ChromaKeyTexture", "Dysfunctional internal GL workings."));
    return std::nullopt;
  }

  auto tex_data = texture->updateTexture(frame_number, eye);
  if (!tex_data) return std::nullopt;
  if (tex_data->frame_number == texture_frame)
    return TextureData{.id = texture_id, //
                       .color = RGB,
                       .frame_number = frame_number};
  texture_frame = frame_number;

  GLint width, height;
  glGetTextureLevelParameteriv(tex_data->id, 0, GL_TEXTURE_WIDTH, &width);
  glGetTextureLevelParameteriv(tex_data->id, 0, GL_TEXTURE_HEIGHT, &height);

  if (width <= 0 || height <= 0) {
    GM_RUNONCE(GM_ERR("ChromaKeyTexture",
                      "Invalid texture size " << width << "x" << height));
    return std::nullopt;
  }

  render_target.push();
  render_target.bind(width, height);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, tex_data->id);

  GLuint program_id = raster_processor.getProgramId();
  glUseProgram(program_id);
  glUniform1i(glGetUniformLocation(program_id, "tex"), 0);
  glUniform3fv(glGetUniformLocation(program_id, "key_rgb"), 1, key.data());
  glUniform1f(glGetUniformLocation(program_id, "r0"), tolerance[0]);
  glUniform1f(glGetUniformLocation(program_id, "r1"), tolerance[1]);

  raster_processor.run();

  glBindTexture(GL_TEXTURE_2D, 0);

  render_target.pop();

  return TextureData{.id = texture_id, //
                     .color = RGB,
                     .frame_number = frame_number};
}

std::optional<TextureInterface::TextureData>
ChromaKeyTexture::updateTexture(size_t frame_number, Eye eye) {
  return _impl->updateTexture(frame_number, eye);
}

void ChromaKeyTexture::setTexture(std::shared_ptr<TextureInterface> texture) {
  _impl->texture = texture;
}

void ChromaKeyTexture::setKey(gmCore::float3 key) {
  _impl->key = key;
}

void ChromaKeyTexture::setTolerance(gmCore::float2 tol) {
  _impl->tolerance = tol;
}

void ChromaKeyTexture::traverse(Visitor *visitor) {
  if (auto obj = std::dynamic_pointer_cast<gmCore::Object>(_impl->texture))
    obj->accept(visitor);
}

END_NAMESPACE_GMGRAPHICS;
