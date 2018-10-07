
#include <gmGraphics/StereoscopicView.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE_ABSTRACT_SUB(StereoscopicView, View);
GM_OFI_PARAM(StereoscopicView, eyeSeparation, float, StereoscopicView::setEyeSeparation);
GM_OFI_POINTER(StereoscopicView, stereoscopicMultiplexer, StereoscopicMultiplexer, StereoscopicView::setStereoscopicMultiplexer);

END_NAMESPACE_GMGRAPHICS;
