
#ifndef GRAMODS_GRAPHICS_CALLBACKRENDERER
#define GRAMODS_GRAPHICS_CALLBACKRENDERER

#include <gmGraphics/Renderer.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   A renderer that defers rendering to a callback function.

   Typical usage:

   ```{c++}
   auto callback = std::make_shared<gmGraphics::CallbackRenderer>();
   callback->setCallback([=](Camera c){ my_render_callback(c); });
   window->addRenderer(callback);
   ```
*/
class CallbackRenderer
  : public Renderer {

public:

  /**
     The signature of a setup function that prepares a renderer.
  */
  typedef std::function<void(void)> SetupFunction;

  /**
     The signature of the rendering function that provides the actual
     graphics. See gmGraphics::Dispatcher for more information.
  */
  typedef std::function<void(Camera)> RenderFunction;

  /**
     Called with GL context to setup renderer.
  */
  void setup() { if (setup_function) setup_function(); }

  /**
     Performs rendering of 3D objects in the scene.
  */
  void render(Camera camera) { if(render_function) render_function(camera); }

  /**
     Sets the callback that should be called upon calls to the render
     method.
  */
  void setCallback(SetupFunction func) { setup_function = func; }

  /**
     Sets the callback that should be called upon calls to the render
     method.
  */
  void setCallback(RenderFunction func) { render_function = func; }

private:

  SetupFunction setup_function;
  RenderFunction render_function;

};

END_NAMESPACE_GMGRAPHICS;

#endif
