
#ifndef GRAMODS_GRAPHICS_TEXTURERENDERER
#define GRAMODS_GRAPHICS_TEXTURERENDERER

#include <gmGraphics/Renderer.hh>
#include <gmGraphics/TextureInterface.hh>

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
  void render(Camera camera, float near = -1, float far = -1);

  /**
     Extracts the currently optimal near and far plane distances. This
     is typically used by a View if there are multiple renderers that
     need to be rendered with the same near and far planes for correct
     depth testing.
  */
  void getNearFar(Camera camera, float &near, float &far);

  /**
     Sets the Texture that should be called upon calls to the render
     method.

     \b XML-attribute: \c texture
  */
  void setTexture(std::shared_ptr<TextureInterface> tex) {
    texture = tex;
  }

  GM_OFI_DECLARE;

private:

  std::shared_ptr<TextureInterface> texture;

  struct Impl;
  std::unique_ptr<Impl> _impl;
};

END_NAMESPACE_GMGRAPHICS;

#endif
