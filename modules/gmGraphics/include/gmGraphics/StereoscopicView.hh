
#ifndef GRAMODS_GRAPHICS_STEREOSCOPICVIEW
#define GRAMODS_GRAPHICS_STEREOSCOPICVIEW

#include <gmGraphics/View.hh>

#include <gmGraphics/StereoscopicMultiplexer.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   The base of Views that support stereoscopic perspectives.
*/
class StereoscopicView
  : public View {

public:

  /**
     The two eyes that can be rendered.
  */
  enum struct Eye {
    MONO,
    LEFT,
    RIGHT
  };

  virtual void renderFullPipeline(ViewSettings settings);

  /**
     Sets the distance between the eyes, in internal units, typically
     meters. Default is 0.06 m.
  */
  void setEyeSeparation(float e) {
    eye_separation = e;
  }

  /**
     Sets the stereoscopic multiplexing to use for rendering. If not
     set, monoscopic rendering will be used.
  */
  void setStereoscopicMultiplexer(std::shared_ptr<StereoscopicMultiplexer> sm) {
    stereoscopic_multiplexer = sm;
  }

  GM_OFI_DECLARE;

protected:

  /**
     Renders one eye at a time, or just a mono eye.
  */
  virtual void renderFullPipeline(ViewSettings settings, Eye eye) = 0;

  std::shared_ptr<StereoscopicMultiplexer> stereoscopic_multiplexer;
  float eye_separation = 0.06f;

};

END_NAMESPACE_GMGRAPHICS;

#endif
