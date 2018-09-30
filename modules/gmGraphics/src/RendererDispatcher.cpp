
#include <gmGraphics/RendererDispatcher.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE(RendererDispatcher);
GM_OFI_POINTER(RendererDispatcher, renderer, gmGraphics::Renderer, RendererDispatcher::addRenderer);
GM_OFI_POINTER(RendererDispatcher, viewpoint, gmGraphics::Viewpoint, RendererDispatcher::setViewpoint);

void RendererDispatcher::renderFullPipeline(ViewSettings settings) {

  settings.renderers.insert(settings.renderers.end(),
                            renderers.begin(), renderers.end());

  Camera c;
  for (auto renderer : settings.renderers)
    renderer->render(c);
}

END_NAMESPACE_GMGRAPHICS;
