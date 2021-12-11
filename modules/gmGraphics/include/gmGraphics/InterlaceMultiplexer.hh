
#ifndef GRAMODS_GRAPHICS_INTERLACEMULTIPLEXER
#define GRAMODS_GRAPHICS_INTERLACEMULTIPLEXER

#include <gmGraphics/MultiscopicMultiplexer.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   Interlaced stereoscopic multiplexer.
*/
class InterlaceMultiplexer
  : public gmGraphics::MultiscopicMultiplexer {

public:

  InterlaceMultiplexer();

  /**
     Sets the interlace pattern between horizontal lines (0), vertical
     lines (1) and checkerboard pattern (2). Default is 0, resulting
     in every other row being left and right eye, respectively.

     \gmXmlTag{gmGraphics,InterlaceMultiplexer,pattern}
  */
  void setPattern(int p);

  /**
     Returns the number of eyes that should be rendered for the
     multiplexer. Returns the value two (2).
  */
  size_t getEyeCount() { return 2; }

  /**
     Prepares the multiplexer for rendering to the two eyes.
  */
  void prepare() override;

  /**
     Sets up rendering for one eye at a time.
  */
  void setupRendering(size_t eye) override;

  /**
     Finalizes the multiplexing.
  */
  void finalize() override;

  GM_OFI_DECLARE;

private:

  struct Impl;
  std::unique_ptr<Impl> _impl;

};

END_NAMESPACE_GMGRAPHICS;

#endif
