
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
     Performs rendering of 3D objects in the scene.
  */
  void render(Camera camera) {
    if (!has_been_setup) {
      if (setup_function) setup_function();
      has_been_setup = true;
    }
    if (render_function) render_function(camera);
  }

  /**
     Sets the callback that should be called to set up GL for
     rendering.
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
  bool has_been_setup = false;

};

END_NAMESPACE_GMGRAPHICS;

#endif
