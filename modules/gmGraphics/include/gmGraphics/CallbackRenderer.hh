
#ifndef GRAMODS_GRAPHICS_CALLBACKRENDERER
#define GRAMODS_GRAPHICS_CALLBACKRENDERER

#include <gmGraphics/Renderer.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   A renderer that defers rendering to a callback function.

   Typical usage:

   ~~~~~{.cpp}
   auto callback = std::make_shared<gmGraphics::CallbackRenderer>();
   callback->setCallback([=](Camera c){ my_render_callback(c); });
   window->addRenderer(callback);
   ~~~~~
*/
class CallbackRenderer
  : public Renderer {

public:

  /**
     The signature of a setup function that prepares a renderer. This
     is called once, the first time the render function is called by
     its view.
  */
  typedef std::function<void(void)> SetupFunction;

  /**
     The signature of the rendering function that provides the actual
     graphics.

     @see gmGraphics::Renderer::render
  */
  typedef std::function<void(const Camera &, const Eigen::Affine3f &)>
      RenderFunction;

  /**
     The signature of an optional function that returns the currently
     optimal near and far plane distances. This is typically used by a
     View if there are multiple renderers that need to be rendered
     with the same near and far planes for correct depth testing.
  */
  typedef std::function<void(
      const Camera &, const Eigen::Affine3f &, float &, float &)>
      NearFarFunction;

  /**
     Performs rendering of 3D objects in the scene.
  */
  void render(const Camera &camera, const Eigen::Affine3f &Mm) override {
    if (!has_been_setup) {
      if (setup_function) setup_function();
      has_been_setup = true;
    }
    if (render_function) render_function(camera, Mm);
  }

  /**
     Extracts the currently optimal near and far plane distances. This
     is typically used by a View if there are multiple renderers that
     need to be rendered with the same near and far planes for correct
     depth testing.
  */
  void getNearFar(const Camera &camera,
                  const Eigen::Affine3f &Mm,
                  float &near,
                  float &far) override {
    if (!nearfar_function) return;
    nearfar_function(camera, Mm, near, far);
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

  /**
     Sets the callback that should be called to get optimal near and
     far plane distances.
  */
  void setCallback(NearFarFunction func) { nearfar_function = func; }

private:

  SetupFunction setup_function;
  RenderFunction render_function;
  NearFarFunction nearfar_function;
  bool has_been_setup = false;

};

END_NAMESPACE_GMGRAPHICS;

#endif
