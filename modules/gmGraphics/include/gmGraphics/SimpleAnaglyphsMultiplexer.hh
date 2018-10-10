
#ifndef GRAMODS_GRAPHICS_SIMPLEANAGLYPHSMULTIPLEXER
#define GRAMODS_GRAPHICS_SIMPLEANAGLYPHSMULTIPLEXER

#include <gmGraphics/StereoscopicMultiplexer.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   Simple anaglyphs implementation.
*/
class SimpleAnaglyphsMultiplexer
  : public gmGraphics::StereoscopicMultiplexer {

public:

  SimpleAnaglyphsMultiplexer();

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
