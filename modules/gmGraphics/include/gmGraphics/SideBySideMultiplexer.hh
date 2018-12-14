
#ifndef GRAMODS_GRAPHICS_SIDEBYSIDEMULTIPLEXER
#define GRAMODS_GRAPHICS_SIDEBYSIDEMULTIPLEXER

#include <gmGraphics/StereoscopicMultiplexer.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   Side-by-side stereoscopic multiplexing implementation.
*/
class SideBySideMultiplexer
  : public gmGraphics::StereoscopicMultiplexer {

public:

  SideBySideMultiplexer();

  /**
     Sets the side-by-side pattern between horizontal (0), vertical
     (1) and frame packing (2). Default is 0.
  */
  void setPattern(int p);

  /**
     Prepares the multiplexer for rendering to the two eyes.
  */
  void prepare();

  /**
     Sets up rendering for one eye at a time.
  */
  void setupRendering(Eye eye);

  /**
     Finalizes the multiplexing.
  */
  void finalize();

  GM_OFI_DECLARE;

private:

  struct Impl;
  std::unique_ptr<Impl> _impl;

};

END_NAMESPACE_GMGRAPHICS;

#endif
