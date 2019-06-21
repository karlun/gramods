
#ifndef GRAMODS_GRAPHICS_TEXTURERENDERER
#define GRAMODS_GRAPHICS_TEXTURERENDERER

#include <gmGraphics/Renderer.hh>
#include <gmGraphics/Texture.hh>

#include <gmCore/OFactory.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   A renderer that renders a texture.
*/
class TextureRenderer
  : public Renderer {

public:

  TextureRenderer();

  /**
     Performs rendering of 3D objects in the scene.
  */
  void render(Camera camera);

  /**
     Sets the Texture that should be called upon calls to the render
     method.

     \b XML-key: \c texture
  */
  void setTexture(std::shared_ptr<Texture> tex) {
    texture = tex;
  }

  GM_OFI_DECLARE;

private:

  std::shared_ptr<Texture> texture;

  struct Impl;
  std::unique_ptr<Impl> _impl;
};

END_NAMESPACE_GMGRAPHICS;

#endif
