
#ifndef GRAMODS_GRAPHICS_MULTISCOPICVIEW
#define GRAMODS_GRAPHICS_MULTISCOPICVIEW

// Required before gmCore/OFactory.hh for some compilers
#include <gmCore/io_eigen.hh>

#include <gmGraphics/View.hh>

#include <gmGraphics/Eye.hh>
#include <gmGraphics/MultiscopicMultiplexer.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   The base of Views that support multiscopic perspectives.
*/
class MultiscopicView
  : public View {

public:

  void renderFullPipeline(ViewSettings settings) override final;

  /**
     Propagates the specified visitor.

     @see Object::Visitor
  */
  void traverse(Visitor *visitor) override;

  /**
     Sets the multiscopic multiplexing to use for rendering. If not
     set, monoscopic rendering will be used.

     \gmXmlTag{gmGraphics,MultiscopicView,multiscopicMultiplexer}
  */
  void setMultiscopicMultiplexer(std::shared_ptr<MultiscopicMultiplexer> sm) {
    multiscopic_multiplexer = sm;
  }

  GM_OFI_DECLARE;

protected:

  /**
     Renders one eye at a time, or just a mono eye.
  */
  virtual void renderFullPipeline(ViewSettings settings, Eye eye) = 0;

  std::shared_ptr<MultiscopicMultiplexer> multiscopic_multiplexer;

};

END_NAMESPACE_GMGRAPHICS;

#endif
