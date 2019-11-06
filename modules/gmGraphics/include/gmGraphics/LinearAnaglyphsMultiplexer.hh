
#ifndef GRAMODS_GRAPHICS_LINEARANAGLYPHSMULTIPLEXER
#define GRAMODS_GRAPHICS_LINEARANAGLYPHSMULTIPLEXER

#include <gmGraphics/MultiscopicMultiplexer.hh>

#include <gmTypes/float.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   An implementation of anaglyphs stereoscopic multiplexing that is
   based on linear operations.
*/
class LinearAnaglyphsMultiplexer
  : public gmGraphics::MultiscopicMultiplexer {

public:

  LinearAnaglyphsMultiplexer();

  /**
     Sets the color of the left eye filter, in RGB 0-1. Default is red
     (1, 0, 0).

     \b XML-attribute: \c leftColor
  */
  void setLeftColor(gmTypes::float3 c);

  /**
     Sets the color of the right eye filter, in RGB 0-1. Default is
     cyan (0, 1, 1).

     \b XML-attribute: \c rightColor
  */
  void setRightColor(gmTypes::float3 c);

  /**
     Sets the saturation of color to use in the final image, in the
     range 0-1. Removing some saturation reduces the problem that an
     object with the same color as the eye filter becomes invisible
     for that eye. Default is 0.8.

     \b XML-attribute: \c saturation
  */
  void setSaturation(float s);

  /**
     Sets the saturation of color to use for the left eye, in the
     range 0-1. Removing some saturation reduces the problem that an
     object with the same color as the eye filter becomes invisible
     for that eye. Default is 0.8.

     \b XML-attribute: \c leftSaturation
  */
  void setLeftSaturation(float s);

  /**
     Sets the saturation of color to use for the right eye, in the
     range 0-1. Removing some saturation reduces the problem that an
     object with the same color as the eye filter becomes invisible
     for that eye. Default is 0.8.

     \b XML-attribute: \c rightSaturation
  */
  void setRightSaturation(float s);

  /**
     Returns the number of eyes that should be rendered for the
     multiplexer. Returns the value two (2).
  */
  size_t getEyeCount() { return 2; }

  /**
     Prepares the multiplexer for rendering to the two eyes.
  */
  void prepare();

  /**
     Sets up rendering for one eye at a time.
  */
  void setupRendering(size_t eye);

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
