
#include <gmGraphics/MultiscopicView.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

GM_OFI_DEFINE_ABSTRACT_SUB(MultiscopicView, View);
GM_OFI_POINTER2(MultiscopicView, multiscopicMultiplexer, MultiscopicMultiplexer, setMultiscopicMultiplexer);

void MultiscopicView::renderFullPipeline(ViewSettings settings) {
  populateViewSettings(settings);

  if (multiscopic_multiplexer) {

    multiscopic_multiplexer->prepare();

    size_t count = multiscopic_multiplexer->getEyeCount();

    for (size_t idx = 0; idx < count; ++idx) {
      multiscopic_multiplexer->setupRendering(idx);
      renderFullPipeline(settings, { idx, count });
    }

    multiscopic_multiplexer->finalize();
    
  } else {
    renderFullPipeline(settings, Eye::MONO);
  }
}

void MultiscopicView::traverse(Visitor *visitor) {
  if (multiscopic_multiplexer) multiscopic_multiplexer->accept(visitor);
}

END_NAMESPACE_GMGRAPHICS;
