
#ifndef GRAMODS_GRAPHICS_QUADBUFFERMULTIPLEXER
#define GRAMODS_GRAPHICS_QUADBUFFERMULTIPLEXER

#include <gmGraphics/StereoscopicMultiplexer.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   Stereo rendering multiplexer that directs left and right eye
   rendering to the left and right back buffers, respectively. This
   requires support for quad buffer rendering on graphics hardware,
   driver and current graphics context.
*/
class QuadBufferMultiplexer
  : public gmGraphics::StereoscopicMultiplexer {

public:

  QuadBufferMultiplexer();

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
