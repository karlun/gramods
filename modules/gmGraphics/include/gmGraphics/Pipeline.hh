
#ifndef GRAMODS_GRAPHICS_PIPELINE
#define GRAMODS_GRAPHICS_PIPELINE

#include <gmGraphics/Dispatcher.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

class Window;

/**
   The base of graphics pipeline implementations.
 */
class Pipeline
  : public Dispatcher {

public:

  /**
     Asks the pipeline to call the specified function once for each
     OpenGL context the current pipeline has defined, for setting up
     internal states.
  */
  void callOnceWithGLContext(Camera::RenderFunction func);

  /**
     Asks the pipeline to call the specified function once for each
     camera that requires rendering for the current pipeline. For a
     pipeline with one window with one tile with simple stereo
     rendering, for example, the specified function will be called
     twice, but more times for full dome rendering.
   */
  void renderFullPipeline(Camera::RenderFunction func);

  /**
     Adds a Window to the pipeline.
  */
  void addWindow(std::shared_ptr<Window> win);

  GM_OFI_DECLARE(Pipeline);

private:

  std::vector<std::shared_ptr<Window>> windows;

};

END_NAMESPACE_GMGRAPHICS;

#endif
