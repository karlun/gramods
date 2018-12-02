
#ifndef GRAMODS_GRAPHICS_INTERLACEMULTIPLEXER
#define GRAMODS_GRAPHICS_INTERLACEMULTIPLEXER

#include <gmGraphics/StereoscopicMultiplexer.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   Simple anaglyphs implementation.
*/
class InterlaceMultiplexer
  : public gmGraphics::StereoscopicMultiplexer {

public:

  InterlaceMultiplexer();

  /**
     Sets the interlace pattern between horizontal lines (0), vertical
     lines (1) and checkerboard pattern (2). Default is 0, resulting
     in every other row being left and right eye, respectively.
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

  class Impl;
  std::unique_ptr<Impl> _impl;

};

END_NAMESPACE_GMGRAPHICS;

#endif
