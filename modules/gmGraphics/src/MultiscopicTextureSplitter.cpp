

#include <gmGraphics/MultiscopicTextureSplitter.hh>

#include <gmGraphics/OffscreenRenderTargets.hh>
#include <gmGraphics/RasterProcessor.hh>

#include <gmCore/RunOnce.hh>
#include <gmCore/Console.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE(MultiscopicTextureSplitter);
GM_OFI_POINTER2(MultiscopicTextureSplitter, texture, TextureInterface, setTexture);
GM_OFI_PARAM2(MultiscopicTextureSplitter, splitType, size_t, setSplitType);

struct MultiscopicTextureSplitter::Impl {

  std::optional<TextureInterface::TextureData>
  updateTexture(size_t frame_number, Eye eye);

  static const std::string fragment_code;

  OffscreenRenderTargets render_target;
  RasterProcessor raster_processor;

  GLuint texture_id = 0;
  bool is_setup = false;
  bool is_functional = false;
  size_t last_update_frame = std::numeric_limits<size_t>::max();

  std::shared_ptr<TextureInterface> texture;

  size_t split_type = 0;
};

const std::string MultiscopicTextureSplitter::Impl::fragment_code =
  R"lang=glsl(
#version 330 core

uniform sampler2D tex;
uniform float r0;
uniform float r1;
uniform float s0;
uniform float s1;

in vec2 position;

out vec4 fragColor;

void main() {
  vec2 t_coord = position.xy * 0.5 + 0.5;
  t_coord.x = t_coord.x * (r1 - r0) + r0;
  t_coord.y = t_coord.y * (s1 - s0) + s0;
  fragColor = texture(tex, t_coord);
}
)lang=glsl";

MultiscopicTextureSplitter::MultiscopicTextureSplitter()
  : _impl(std::make_unique<Impl>()) {}

std::optional<TextureInterface::TextureData>
MultiscopicTextureSplitter::Impl::updateTexture(size_t frame_number, Eye eye) {

  if (!texture) {
    GM_RUNONCE(GM_ERR("MultiscopicTextureSplitter", "No texture to process."));
    return std::nullopt;
  }

  if (split_type > 1) {
    GM_RUNONCE(GM_ERR("MultiscopicTextureSplitter", "Unknown split type " << split_type));
    return std::nullopt;
  }

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
    GM_RUNONCE(GM_ERR("MultiscopicTextureSplitter", "Dysfunctional internal GL workings."));
    return std::nullopt;
  }
  
  const auto t_data = texture->updateTexture(frame_number, eye);
  if (!t_data) return std::nullopt;

  if (t_data->frame_number == last_update_frame)
    return TextureData{.id = texture_id, //
                        .color = RGB,
                        .frame_number = last_update_frame};
  last_update_frame = frame_number;

  GLint width, height;
  glGetTextureLevelParameteriv(t_data->id, 0, GL_TEXTURE_WIDTH, &width);
  glGetTextureLevelParameteriv(t_data->id, 0, GL_TEXTURE_HEIGHT, &height);

  if (width <= 0 || height <= 0) {
    GM_RUNONCE(GM_ERR("MultiscopicTextureSplitter",
                      "Invalid texture size " << width << "x" << height));
    return std::nullopt;
  }

  switch(split_type) {
  case 0:
    render_target.push();
    render_target.bind(width / 2, height);
    break;
  case 1:
    render_target.push();
    render_target.bind(width, height / 2);
    break;
  default:
    throw gmCore::InvalidArgument(
        GM_STR("Unsupported split type " << split_type));
  }

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, t_data->id);

  eye.validate();
  GLuint program_id = raster_processor.getProgramId();
  glUseProgram(program_id);
  glUniform1i(glGetUniformLocation(program_id, "tex"), 0);
  switch(split_type){
  case 0:
    glUniform1f(glGetUniformLocation(program_id, "r0"),
                (eye.idx + 0) / float(eye.count));
    glUniform1f(glGetUniformLocation(program_id, "r1"),
                (eye.idx + 1) / float(eye.count));
    glUniform1f(glGetUniformLocation(program_id, "s0"), 0.f);
    glUniform1f(glGetUniformLocation(program_id, "s1"), 1.f);
    break;
  case 1:
    glUniform1f(glGetUniformLocation(program_id, "r0"), 0.f);
    glUniform1f(glGetUniformLocation(program_id, "r1"), 1.f);
    glUniform1f(glGetUniformLocation(program_id, "s0"),
                (eye.count - eye.idx - 1) / float(eye.count));
    glUniform1f(glGetUniformLocation(program_id, "s1"),
                (eye.count - eye.idx) / float(eye.count));
  }

  raster_processor.run();

  glBindTexture(GL_TEXTURE_2D, 0);

  render_target.pop();

  return TextureData{.id = texture_id, //
                     .color = RGB,
                     .frame_number = last_update_frame};
}

std::optional<TextureInterface::TextureData>
MultiscopicTextureSplitter::updateTexture(size_t frame_number, Eye eye) {
  return _impl->updateTexture(frame_number, eye);
}

void MultiscopicTextureSplitter::setTexture(std::shared_ptr<TextureInterface> texture) {
  _impl->texture = texture;
}

void MultiscopicTextureSplitter::setSplitType(size_t type) {
  if (type > 1)
    throw gmCore::InvalidArgument(GM_STR("Unsupported split type " << type));
  _impl->split_type = type;
}

void MultiscopicTextureSplitter::traverse(Visitor *visitor) {
  if (auto obj = std::dynamic_pointer_cast<gmCore::Object>(_impl->texture))
    obj->accept(visitor);
}

END_NAMESPACE_GMGRAPHICS;
