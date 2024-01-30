
#include <gmGraphics/RendererDispatcher.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE(RendererDispatcher);
GM_OFI_POINTER2(RendererDispatcher, renderer, gmGraphics::Renderer, addRenderer);
GM_OFI_POINTER2(RendererDispatcher, viewpoint, gmGraphics::Viewpoint, setViewpoint);

void RendererDispatcher::renderFullPipeline(ViewSettings settings) {

  populateViewSettings(settings);

  Camera camera(settings);
  float near, far;
  Renderer::getNearFar(settings.renderers, camera, near, far);

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  for (auto renderer : settings.renderers)
    renderer->render(camera, near, far);
}

void RendererDispatcher::populateViewSettings(ViewSettings &settings) {
  settings.renderers.insert(settings.renderers.end(),
                            renderers.begin(), renderers.end());
  if (viewpoint)
    settings.viewpoint = viewpoint;
}

void RendererDispatcher::clearRenderers(bool) {
  renderers.clear();
}

END_NAMESPACE_GMGRAPHICS;
