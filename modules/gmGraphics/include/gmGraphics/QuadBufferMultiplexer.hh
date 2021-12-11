
#ifndef GRAMODS_GRAPHICS_QUADBUFFERMULTIPLEXER
#define GRAMODS_GRAPHICS_QUADBUFFERMULTIPLEXER

#include <gmGraphics/MultiscopicMultiplexer.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   Stereo rendering multiplexer that directs left and right eye
   rendering to the left and right back buffers, respectively. This
   requires support for quad buffer rendering on graphics hardware,
   driver and current graphics context.
*/
class QuadBufferMultiplexer
  : public gmGraphics::MultiscopicMultiplexer {

public:

  QuadBufferMultiplexer();

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
