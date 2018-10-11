
#ifndef GRAMODS_GRAPHICS_STEREOSCOPICMULTIPLEXER
#define GRAMODS_GRAPHICS_STEREOSCOPICMULTIPLEXER

#include <gmGraphics/config.hh>

#include <gmTypes/all.hh>
#include <gmCore/Object.hh>
#include <gmCore/OFactory.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   The stereoscopic multiplexing is the process of rendering images
   for the left and the right eye in a format that can be physically
   or optically separated at a later stage so that these images are
   visible only to the correct eye. Examples are sequential frame
   multiplex, split screen or pixel interlaced multiplexing, and
   anaglyphic color multiplexing.

   A StereoscopicView will for each frame call, in order,

    1. prepare()
    2. setupRendering(Eye::LEFT)
    3. setupRendering(Eye::RIGHT)
    4. finalize()
*/
class StereoscopicMultiplexer
  : public gmCore::Object {

public:

  /**
     The two eyes that can be rendered.
  */
  enum struct Eye {
    LEFT = 0,
    RIGHT,
    COUNT
  };

  /**
     Prepares the multiplexer for rendering to the two eyes.
  */
  virtual void prepare() = 0;

  /**
     Sets up rendering for one eye at a time.
  */
  virtual void setupRendering(Eye eye) = 0;

  /**
     Finalizes the multiplexing.
  */
  virtual void finalize() = 0;

};

END_NAMESPACE_GMGRAPHICS;

#endif
