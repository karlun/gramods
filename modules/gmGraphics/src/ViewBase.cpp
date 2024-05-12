
#include <gmGraphics/ViewBase.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE(ViewBase);
GM_OFI_POINTER2(ViewBase, renderer, gmGraphics::Renderer, addRenderer);
GM_OFI_POINTER2(ViewBase, viewpoint, gmGraphics::Viewpoint, addViewpoint);

void ViewBase::renderFullPipeline(ViewSettings settings) {

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

void ViewBase::populateViewSettings(ViewSettings &settings) {
  settings.renderers.insert(settings.renderers.end(),
                            renderers.begin(), renderers.end());
  if (!viewpoints.empty())
    settings.viewpoints.insert(
        settings.viewpoints.end(), viewpoints.begin(), viewpoints.end());
}

void ViewBase::clearRenderers(bool) {
  renderers.clear();
}

void ViewBase::traverse(Visitor *visitor) {
  for (auto &r : renderers) r->accept(visitor);
  for (auto &v : viewpoints) v->accept(visitor);
}

END_NAMESPACE_GMGRAPHICS;
