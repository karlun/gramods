
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

  /**
     Sets the distance between the eyes, in internal units, typically
     meters.
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

  std::shared_ptr<StereoscopicMultiplexer> stereoscopic_multiplexer;
  float eye_separation = 0.65;

};

END_NAMESPACE_GMGRAPHICS;

#endif
