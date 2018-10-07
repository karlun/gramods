
#include <gmGraphics/RendererDispatcher.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE(RendererDispatcher);
GM_OFI_POINTER(RendererDispatcher, renderer, gmGraphics::Renderer, RendererDispatcher::addRenderer);
GM_OFI_POINTER(RendererDispatcher, viewpoint, gmGraphics::Viewpoint, RendererDispatcher::setViewpoint);

void RendererDispatcher::renderFullPipeline(ViewSettings settings) {

  populateViewSettings(settings);

  Camera camera;
  for (auto renderer : settings.renderers)
    renderer->render(camera);
}

void RendererDispatcher::populateViewSettings(ViewSettings &settings) {
  settings.renderers.insert(settings.renderers.end(),
                            renderers.begin(), renderers.end());
  if (viewpoint)
    settings.viewpoint = viewpoint;
}

END_NAMESPACE_GMGRAPHICS;
