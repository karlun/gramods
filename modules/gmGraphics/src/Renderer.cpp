
#include <gmGraphics/Renderer.hh>

#include <limits>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE_ABSTRACT(Renderer);
GM_OFI_PARAM2(Renderer, eye, gmGraphics::Eye, addEye);

void Renderer::addEye(Eye e) {
  eyes.insert(e);
}

END_NAMESPACE_GMGRAPHICS;
