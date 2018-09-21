
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

  /**
     Called with GL context to setup renderer.
  */
  virtual void setup() = 0;

  /**
     Performs rendering of 3D objects in the scene.
  */
  virtual void render(Camera camera) = 0;

};

END_NAMESPACE_GMGRAPHICS;

#endif
