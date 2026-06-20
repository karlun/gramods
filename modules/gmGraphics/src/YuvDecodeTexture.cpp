

#include <gmGraphics/YuvDecodeTexture.hh>

#include <gmGraphics/OffscreenRenderTargets.hh>
#include <gmGraphics/RasterProcessor.hh>

#include <gmCore/RunOnce.hh>
#include <gmCore/Console.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE(YuvDecodeTexture);
GM_OFI_POINTER2(YuvDecodeTexture, texture, Texture, setTexture);
GM_OFI_PARAM2(YuvDecodeTexture, yuvToRgb, Eigen::Matrix3f, setYuvToRgb);
GM_OFI_PARAM2(YuvDecodeTexture, uvRange, gmCore::float2, setUvRange);

struct YuvDecodeTexture::Impl {

  std::optional<TextureInterface::TextureData>
  updateTexture(size_t frame_number, Eye eye);

  static const std::string fragment_code;

  OffscreenRenderTargets render_target;
  RasterProcessor raster_processor;

  GLuint texture_id = 0;
  size_t texture_frame = std::numeric_limits<size_t>::max();
  bool is_setup = false;
  bool is_functional = false;

  std::shared_ptr<Texture> texture;
  Eigen::Matrix3f yuvToRgb = (Eigen::Matrix3f() <<
                              1.f,  0.00000f,  1.13983f,
                              1.f, -0.39465f, -0.58060f,
                              1.f,  2.03211f,  0.00000f).finished();
  Eigen::Vector3f yuvRange = Eigen::Vector3f(1.f, 0.436f, 0.615f);
};

const std::string YuvDecodeTexture::Impl::fragment_code =
  R"lang=glsl(
#version 330 core

uniform sampler2D tex;
uniform mat3 yuvToRgb;
uniform vec3 yuvRange;

in vec2 position;

out vec4 fragColor;

void main() {

  ivec2 size = textureSize(tex, 0) * ivec2(2, 1);

  ivec2 pix = ivec2((0.5 + 0.5 * position) * size);
  vec4 yuyv = texelFetch(tex, pix / ivec2(2, 1), 0);

  vec3 yuv;
  if (pix.x % 2 == 0) {
    yuv = yuvRange * (yuyv.xyw * vec3(1, 2, 2) - vec3(0, 1, 1));
  } else {
    yuv = yuvRange * (yuyv.zyw * vec3(1, 2, 2) - vec3(0, 1, 1));
  }

  vec3 rgb = yuvToRgb * yuv;
  fragColor = vec4(rgb, 1);
}
)lang=glsl";
    
YuvDecodeTexture::YuvDecodeTexture()
  : _impl(std::make_unique<Impl>()) {}

std::optional<TextureInterface::TextureData>
YuvDecodeTexture::Impl::updateTexture(size_t frame_number, Eye eye) {

  if (!texture) {
    GM_RUNONCE(GM_ERR("YuvDecodeTexture", "No texture to decode."));
    return std::nullopt;
  }

  auto tex_data = texture->updateTexture(frame_number, eye);
  if (!tex_data) return std::nullopt;

  if (texture_frame == tex_data->frame_number)
    return TextureData{.id = texture_id, //
                       .color = RGB,
                       .frame_number = texture_frame};
  texture_frame = tex_data->frame_number;

  if (!is_setup) {
    is_setup = true;
    raster_processor.setFragmentCode(fragment_code);
    render_target.setLinearInterpolation(false);
    if (render_target.init(1) &&
        raster_processor.init()) {
      is_functional = true;
      texture_id = render_target.getTexId(0);
    }
  }

  if (!is_functional) {
    GM_RUNONCE(GM_ERR("YuvDecodeTexture", "Dysfunctional internal GL workings."));
    return std::nullopt;
  }

  GLint width, height;
  glGetTextureLevelParameteriv(tex_data->id, 0, GL_TEXTURE_WIDTH, &width);
  glGetTextureLevelParameteriv(tex_data->id, 0, GL_TEXTURE_HEIGHT, &height);

  if (width <= 0 || height <= 0) {
    GM_RUNONCE(GM_ERR("YuvDecodeTexture", "Invalid texture size " << width << "x" << height));
    return std::nullopt;
  }

  render_target.push();
  render_target.bind(2 * width, height);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, tex_data->id);

  GLuint program_id = raster_processor.getProgramId();
  glUseProgram(program_id);
  glUniform1i(glGetUniformLocation(program_id, "tex"), 0);
  glUniformMatrix3fv(glGetUniformLocation(program_id, "yuvToRgb"), 1, false, yuvToRgb.data());
  glUniform3fv(glGetUniformLocation(program_id, "yuvRange"), 1, yuvRange.data());

  raster_processor.run();

  glBindTexture(GL_TEXTURE_2D, 0);

  render_target.pop();

  return TextureData{.id = texture_id, //
                     .color = RGB,
                     .frame_number = texture_frame};
}

std::optional<TextureInterface::TextureData>
YuvDecodeTexture::updateTexture(size_t frame_number, Eye eye) {
  return _impl->updateTexture(frame_number, eye);
}

void YuvDecodeTexture::setTexture(std::shared_ptr<Texture> texture) {
  _impl->texture = texture;
}

void YuvDecodeTexture::setYuvToRgb(Eigen::Matrix3f m) {
  _impl->yuvToRgb = m;
}

void YuvDecodeTexture::setUvRange(gmCore::float2 v) {
  _impl->yuvRange = Eigen::Vector3f(1, v[0], v[1]);
}

void YuvDecodeTexture::traverse(Visitor *visitor) {
  if (_impl->texture) _impl->texture->accept(visitor);
}

END_NAMESPACE_GMGRAPHICS;
