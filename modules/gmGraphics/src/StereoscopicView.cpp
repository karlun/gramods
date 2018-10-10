
#include <gmGraphics/StereoscopicView.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE_ABSTRACT_SUB(StereoscopicView, View);
GM_OFI_PARAM(StereoscopicView, eyeSeparation, float, StereoscopicView::setEyeSeparation);
GM_OFI_POINTER(StereoscopicView, stereoscopicMultiplexer, StereoscopicMultiplexer, StereoscopicView::setStereoscopicMultiplexer);

void StereoscopicView::renderFullPipeline(ViewSettings settings) {
  populateViewSettings(settings);

  if (stereoscopic_multiplexer) {

    stereoscopic_multiplexer->prepare();

    stereoscopic_multiplexer->setupRendering(StereoscopicMultiplexer::Eye::LEFT);
    renderFullPipeline(settings, Eye::LEFT);

    stereoscopic_multiplexer->setupRendering(StereoscopicMultiplexer::Eye::RIGHT);
    renderFullPipeline(settings, Eye::RIGHT);

    stereoscopic_multiplexer->finalize();
    
  } else {
    renderFullPipeline(settings, Eye::MONO);
  }
}

END_NAMESPACE_GMGRAPHICS;
