
#ifndef GRAMODS_GRAPHICS_MULTISCOPICMULTIPLEXER
#define GRAMODS_GRAPHICS_MULTISCOPICMULTIPLEXER

#include <gmGraphics/config.hh>

#include <gmCore/Object.hh>
#include <gmCore/OFactory.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   The multiscopic multiplexing is the process of rendering images for
   the left and the right (or more) eyes in a format that can be
   physically or optically separated at a later stage so that these
   images are visible only to the correct eye. Examples are sequential
   frame multiplex, split screen or pixel interlaced multiplexing, and
   anaglyphic color multiplexing.

   A MultiscopicView will for each frame call, in order,

    1. prepare()
    2. N = getEyeCount()
    3. setupRendering(0)
    4. ...
    5. setupRendering(N-1)
    6. finalize()
*/
class MultiscopicMultiplexer
  : public gmCore::Object {

public:

  /**
     Returns the number of eyes that should be rendered for the
     multiplexer. Most multiplexers are stereoscopic and will
     therefore return two (2).
  */
  virtual size_t getEyeCount() = 0;

  /**
     Prepares the multiplexer for rendering to the two eyes.
  */
  virtual void prepare() = 0;

  /**
     Sets up rendering for one eye at a time.
  */
  virtual void setupRendering(size_t eye) = 0;

  /**
     Finalizes the multiplexing.
  */
  virtual void finalize() = 0;

  /**
     Returns the default key, in Configuration, for the
     Object.
  */
  virtual std::string getDefaultKey() override { return "multiscopicMultiplexer"; }

};

END_NAMESPACE_GMGRAPHICS;

#endif
