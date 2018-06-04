
#ifndef GRAMODS_GRAPHICS_PIPELINE
#define GRAMODS_GRAPHICS_PIPELINE

#include <gmGraphics/config.hh>

#include <gmCore/Object.hh>

#include <functional>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   The base of graphics pipeline implementations.
 */
class Pipeline
  : public gmCore::Object {

public:

  struct Camera {
    int GLContextIdx;
  };

  typedef std::function<void(Camera)> RenderFunction;

  /**
     Asks the pipeline to call the specified function once for each
     OpenGL context the current pipeline has defined, for setting up
     internal states.
  */
  virtual void callOnceWithGLContext(RenderFunction func) = 0;

  /**
     Asks the pipeline to call the specified function once for each
     camera that requires rendering for the current pipeline. For
     simple stereo rendering, for example, the specified function will
     be called twice, but more times for full dome rendering.
   */
  virtual void renderFullPipeline(RenderFunction func) = 0;

};

END_NAMESPACE_GMGRAPHICS;

#endif
