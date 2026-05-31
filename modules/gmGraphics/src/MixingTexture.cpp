
#include <gmGraphics/MixingTexture.hh>

#include <gmGraphics/GLUtils.hh>
#include <gmCore/RunOnce.hh>

#include <gmGraphics/MixingShaders.hh>
#include <gmGraphics/OffscreenRenderTargets.hh>
#include <gmGraphics/RasterProcessor.hh>

#include <Eigen/Eigen>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE(MixingTexture);
GM_OFI_PARAM2(MixingTexture, mixType, std::string, setMixType);
GM_OFI_POINTER2(MixingTexture, texture, TextureInterface, addTexture);

struct MixingTexture::Impl {

  void update(size_t frame_number, Eye e);

  std::string mix_type = "average";

  OffscreenRenderTargets render_target;
  RasterProcessor raster_processor;

  GLuint texture_id = 0;
  bool is_setup = false;
  bool is_functional = false;

  std::vector<std::shared_ptr<TextureInterface>> textures;
};

MixingTexture::MixingTexture() : _impl(std::make_unique<Impl>()) {}

MixingTexture::~MixingTexture() {}

void MixingTexture::addTexture(std::shared_ptr<TextureInterface> v) {
  if (!v) throw gmCore::InvalidArgument("null not allowed");
  _impl->textures.push_back(v);
}

void MixingTexture::setMixType(std::string str) {

  if (_impl->is_setup)
    GM_WRN("MixingTexture",
           "Settings mix type after initialization will have no effect.");

  if (MixingShaders::checkMixTypeValid(str, "MixingTexture"))
    _impl->mix_type = str;
}

GLuint MixingTexture::updateTexture(size_t frame_number, Eye eye) {
  _impl->update(frame_number, eye);
  return _impl->texture_id;
}

void MixingTexture::Impl::update(size_t frame_number, Eye eye) {

  if (textures.empty()) {
    GM_RUNONCE(GM_ERR("MixingTexture", "Missing textures to mix."));
    return;
  }

  if (!MixingShaders::checkCount(
          mix_type, textures.size(), "MixingTexture", "textures"))
    return;

  if (!is_setup) {
    is_setup = true;
    raster_processor.setFragmentCode(MixingShaders::getFragmentCode(mix_type));
    render_target.setLinearInterpolation(false);

    auto tex_id = textures.back()->updateTexture(frame_number, eye);

    glBindTexture(GL_TEXTURE_2D, tex_id);
    GLint format = 0;
    glGetTexLevelParameteriv(
        GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &format);
    render_target.setPixelFormat(format);
    glBindTexture(GL_TEXTURE_2D, 0);
    GM_DBG1("MixingTexture", "Setting up with " << GLUtils::toString(format));

    if (render_target.init(1) && raster_processor.init()) {
      is_functional = true;
      texture_id = render_target.getTexId(0);
    }
  }

  if (!is_functional) {
    GM_RUNONCE(GM_ERR("MixingTexture", "Dysfunctional internal GL workings."));
    return;
  }

  GM_DBG2("MixingTexture", "Render " << textures.size() << " textures.");
  std::vector<GLuint> tex_id;
  tex_id.reserve(textures.size());

  for (auto texture : textures)
    tex_id.push_back(texture->updateTexture(frame_number, eye));

  GLint width, height;
  glGetTextureLevelParameteriv(tex_id.back(), 0, GL_TEXTURE_WIDTH, &width);
  glGetTextureLevelParameteriv(tex_id.back(), 0, GL_TEXTURE_HEIGHT, &height);

  if (width <= 0 || height <= 0) {
    GM_RUNONCE(GM_ERR("ChromaKeyTexture",
                      "Invalid texture size " << width << "x" << height));
    return;
  }

  // Render offscreen buffer to active render target

  render_target.push();
  render_target.bind(width, height);

  GM_DBG2("MixingTexture", "Render offscreen buffers to active render target");

  for (size_t idx = 0; idx < textures.size(); ++idx) {
    glActiveTexture(GL_TEXTURE0 + idx);
    glBindTexture(GL_TEXTURE_2D, tex_id[idx]);
  }

  GLuint program_id = raster_processor.getProgramId();
  glUseProgram(program_id);
  static const GLint tex[] = {0, 1, 2, 3, 4, 5, 6, 7};
  glUniform1iv(glGetUniformLocation(program_id, "tex"), 8, tex);
  glUniform1i(glGetUniformLocation(program_id, "tex_count"), textures.size());

  raster_processor.run();

  for (size_t idx = 0; idx < textures.size(); ++idx) {
    glActiveTexture(GL_TEXTURE0 + idx);
    glBindTexture(GL_TEXTURE_2D, 0);
  }

  render_target.pop();
}

void MixingTexture::traverse(Visitor *visitor) {
  for (auto &v : _impl->textures)
    if (auto obj = std::dynamic_pointer_cast<gmCore::Object>(v))
      obj->accept(visitor);
}

END_NAMESPACE_GMGRAPHICS;
