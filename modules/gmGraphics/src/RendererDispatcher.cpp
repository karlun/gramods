
#include <gmGraphics/RendererDispatcher.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE(RendererDispatcher);
GM_OFI_POINTER(RendererDispatcher, renderer, gmGraphics::Renderer, RendererDispatcher::addRenderer);
GM_OFI_POINTER(RendererDispatcher, viewpoint, gmGraphics::Viewpoint, RendererDispatcher::setViewpoint);

void RendererDispatcher::renderFullPipeline(ViewSettings settings) {

  if (renderers_to_setup.empty()) return;

  GM_VINF("Window", "setting up " << renderers_to_setup.size() << " renderers");
  while (!renderers_to_setup.empty()) {
    auto &renderer = renderers_to_setup.back();
    renderer->setup();
    renderers.push_back(renderer);
    renderers_to_setup.pop_back();
  }
}

END_NAMESPACE_GMGRAPHICS;
