
#ifndef GRAMODS_GRAPHICS_CUBESCENERENDERER
#define GRAMODS_GRAPHICS_CUBESCENERENDERER

#include <gmGraphics/Renderer.hh>

#include <gmTypes/all.hh>
#include <gmCore/OFactory.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   A renderer that draws a set of cubes, for demonstration and testing
   purposes.
*/
class CubeSceneRenderer
  : public Renderer {

public:

  CubeSceneRenderer();

  /**
     Performs rendering of 3D objects in the scene.
  */
  void render(Camera camera);

  void setCubeSize(float d);
  void setCubeSetSize(float d);
  void setCubeSetCenter(gmTypes::float3 c);

  GM_OFI_DECLARE;

private:

  class Impl;
  std::unique_ptr<Impl> _impl;

};

END_NAMESPACE_GMGRAPHICS;

#endif
