
#ifndef GRAMODS_GRAPHICS_RENDERER
#define GRAMODS_GRAPHICS_RENDERER

#include <gmGraphics/config.hh>

#include <gmCore/Object.hh>
#include <gmGraphics/Camera.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   The base of graphics Renderer implementations.
 */
class Renderer
  : public gmCore::Object {

public:

  typedef std::vector<std::shared_ptr<Renderer>> list;

  /**
     Performs rendering of 3D objects in the scene.
  */
  virtual void render(Camera camera, float near = -1, float far = -1) = 0;

  /**
     Extracts the currently optimal near and far plane distances. This
     is typically used by a View if there are multiple renderers that
     need to be rendered with the same near and far planes for correct
     depth testing.
  */
  virtual void getNearFar(Camera camera, float &near, float &far) = 0;

  /**
     Returns the default key, in Configuration, for the
     Object.
  */
  virtual std::string getDefaultKey() override { return "renderer"; }

  /**
     Finds good near and far plane distances from the optimal near and
     far planes for the specified renderers. Call this from overloaded
     renderFullPipeline.
  */
  static void getNearFar(Renderer::list renderers, Camera camera,
                         float &near, float &far);

};

END_NAMESPACE_GMGRAPHICS;

#endif
