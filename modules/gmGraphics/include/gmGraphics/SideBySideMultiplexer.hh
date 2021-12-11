
#ifndef GRAMODS_GRAPHICS_SIDEBYSIDEMULTIPLEXER
#define GRAMODS_GRAPHICS_SIDEBYSIDEMULTIPLEXER

#include <gmGraphics/MultiscopicMultiplexer.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   Side-by-side stereoscopic multiplexing implementation.
*/
class SideBySideMultiplexer
  : public gmGraphics::MultiscopicMultiplexer {

public:

  SideBySideMultiplexer();

  /**
     Sets the side-by-side pattern between horizontal (0), vertical
     (1) and frame packing (2). Default is 0.

     \gmXmlTag{gmGraphics,SideBySideMultiplexer,pattern}
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
