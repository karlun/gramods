
#ifndef GRAMODS_GRAPHICS_DISPATCHER
#define GRAMODS_GRAPHICS_DISPATCHER

#include <gmGraphics/config.hh>

#include <gmCore/Object.hh>
#include <gmCore/OFactory.hh>
#include <gmGraphics/Camera.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   The base of dispatcher implementations, classes with the role of
   calling renderers, or other dispatchers.
 */
class Dispatcher
  : public gmCore::Object {

  /**
     Asks the Dispatcher to call the specified function once for each
     OpenGL context in the current setup, for setting up internal
     states.
  */
  virtual void callOnceWithGLContext(Camera::RenderFunction func) = 0;

  /**
     Asks the Dispatcher to call the specified function once for each
     camera that requires rendering for the current setup. For simple
     stereo rendering, for example, the specified function will be
     called twice, but more times for full dome rendering.
   */
  virtual void renderFullPipeline(Camera::RenderFunction func) = 0;

};

END_NAMESPACE_GMGRAPHICS;

#endif
