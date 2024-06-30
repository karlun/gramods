
#include <gmGraphics/MultiscopicTextureMultiplexer.hh>

#include <gmCore/RunOnce.hh>
#include <gmCore/Console.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE(MultiscopicTextureMultiplexer);
GM_OFI_POINTER2(MultiscopicTextureMultiplexer, texture, TextureInterface, addTexture);

struct MultiscopicTextureMultiplexer::Impl {

  GLuint updateTexture(size_t frame_number, Eye eye);
  std::vector<std::shared_ptr<TextureInterface>> textures;

};

MultiscopicTextureMultiplexer::MultiscopicTextureMultiplexer()
  : _impl(std::make_unique<Impl>()) {}


GLuint MultiscopicTextureMultiplexer::updateTexture(size_t frame_number, Eye eye) {
  return _impl->updateTexture(frame_number, eye);
}

GLuint MultiscopicTextureMultiplexer::Impl::updateTexture(size_t frame_number, Eye eye) {
  if (eye.count != textures.size()) {
    GM_RUNONCE(GM_ERR("MultiscopicTextureMultiplexer",
                      "Number of multiscopic views ("
                          << eye.count << ") does not match number of textures ("
                          << textures.size() << ")"));
  }
  if (eye.idx >= textures.size()) {
    GM_RUNONCE(GM_ERR("MultiscopicTextureMultiplexer",
                      "No texture available for multiscopic view "
                          << eye.idx << " (" << textures.size()
                          << " available)"));
    return 0;
  }

  return textures[eye.idx]->updateTexture(frame_number, eye);
}

void MultiscopicTextureMultiplexer::addTexture(std::shared_ptr<TextureInterface> texture) {
  if (!texture) throw gmCore::InvalidArgument("null not allowed");
  _impl->textures.push_back(texture);
}

void MultiscopicTextureMultiplexer::traverse(Visitor *visitor) {
  for (auto &t : _impl->textures)
    if (auto obj = std::dynamic_pointer_cast<gmCore::Object>(t))
      obj->accept(visitor);
}

END_NAMESPACE_GMGRAPHICS;
