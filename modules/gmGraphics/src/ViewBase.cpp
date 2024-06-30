
#include <gmGraphics/ViewBase.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE(ViewBase);
GM_OFI_POINTER2(ViewBase, node, gmGraphics::Node, addNode);
GM_OFI_POINTER2(ViewBase, viewpoint, gmGraphics::Viewpoint, addViewpoint);

void ViewBase::renderFullPipeline(ViewSettings settings) {

  populateViewSettings(settings);

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  Camera camera(settings);
  settings.renderNodes(camera);
}

void ViewBase::ViewSettings::renderNodes(Camera camera) {
  Node::NearFarVisitor nf_visitor(camera);
  for (auto &node : nodes) node->accept(&nf_visitor);

  auto near_far = nf_visitor.getNearFar();
  if (!near_far) {
    GM_DBG2("ViewSettings", "Abort rendering: invalid near-far planes");
    return;
  }

  GM_DBG2("ViewSettings",
          "Rendering with near-far " << near_far->first << " - "
                                     << near_far->second);

  camera.setNearFar(near_far->first, near_far->second);
  Node::RenderVisitor r_visitor(camera);
  for (auto &node : nodes) node->accept(&r_visitor);
}

void ViewBase::populateViewSettings(ViewSettings &settings) {
  settings.nodes.insert(settings.nodes.end(), nodes.begin(), nodes.end());
  if (!viewpoints.empty())
    settings.viewpoints.insert(
        settings.viewpoints.end(), viewpoints.begin(), viewpoints.end());
}

void ViewBase::traverse(Visitor *visitor) {
  for (auto &n : nodes) n->accept(visitor);
  for (auto &v : viewpoints) v->accept(visitor);
}

END_NAMESPACE_GMGRAPHICS;
