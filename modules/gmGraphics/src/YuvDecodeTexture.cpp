

#include <gmGraphics/YuvDecodeTexture.hh>

#include <gmGraphics/OffscreenRenderTargets.hh>
#include <gmGraphics/RasterProcessor.hh>

#include <gmCore/RunOnce.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE(YuvDecodeTexture);
GM_OFI_POINTER(YuvDecodeTexture, texture, Texture, YuvDecodeTexture::setTexture);
GM_OFI_PARAM(YuvDecodeTexture, yuvToRgb, Eigen::Matrix3f, YuvDecodeTexture::setYuvToRgb);
GM_OFI_PARAM(YuvDecodeTexture, uvRange, gmTypes::float2, YuvDecodeTexture::setUvRange);

struct YuvDecodeTexture::Impl {

  void update();
  GLuint getGLTextureID() { return texture_id; }

  static const std::string fragment_code;

  OffscreenRenderTargets render_target;
  RasterProcessor raster_processor;

  GLuint texture_id = 0;
  bool is_setup = false;
  bool is_functional = false;

  std::shared_ptr<Texture> texture;
  Eigen::Matrix3f yuvToRgb = (Eigen::Matrix3f() <<
                              1,  0.00000,  1.13983,
                              1, -0.39465, -0.58060,
                              1,  2.03211,  0.00000).finished();
  Eigen::Vector3f yuvRange = Eigen::Vector3f(1, 0.436, 0.615);
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

void YuvDecodeTexture::update() {
  _impl->update();
}

void YuvDecodeTexture::Impl::update() {

  if (!texture) {
    GM_RUNONCE(GM_ERR("YuvDecodeTexture", "No texture to decode."));
    return;
  }

  texture->update();

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
    return;
  }

  GLuint tex_id = texture->getGLTextureID();

  GLint width, height;
  glGetTextureLevelParameteriv(tex_id, 0, GL_TEXTURE_WIDTH, &width);
  glGetTextureLevelParameteriv(tex_id, 0, GL_TEXTURE_HEIGHT, &height);

  if (width <= 0 || height <= 0) {
    GM_RUNONCE(GM_ERR("YuvDecodeTexture", "Invalid texture size " << width << "x" << height));
    return;
  }

  render_target.push();
  render_target.bind(2 * width, height);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, tex_id);

  GLuint program_id = raster_processor.getProgramId();
  glUseProgram(program_id);
  glUniform1i(glGetUniformLocation(program_id, "tex"), 0);
  glUniformMatrix3fv(glGetUniformLocation(program_id, "yuvToRgb"), 1, false, yuvToRgb.data());
  glUniform3fv(glGetUniformLocation(program_id, "yuvRange"), 1, yuvRange.data());

  raster_processor.run();

  glBindTexture(GL_TEXTURE_2D, 0);

  render_target.pop();
}

GLuint YuvDecodeTexture::getGLTextureID() {
  return _impl->getGLTextureID();
}

void YuvDecodeTexture::setTexture(std::shared_ptr<Texture> texture) {
  _impl->texture = texture;
}

void YuvDecodeTexture::setYuvToRgb(Eigen::Matrix3f m) {
  _impl->yuvToRgb = m;
}

void YuvDecodeTexture::setUvRange(gmTypes::float2 v) {
  _impl->yuvRange = Eigen::Vector3f(1, v[0], v[1]);
}


END_NAMESPACE_GMGRAPHICS;
