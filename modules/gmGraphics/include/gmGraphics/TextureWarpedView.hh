
#ifndef GRAMODS_GRAPHICS_TEXTUREWARPEDVIEW
#define GRAMODS_GRAPHICS_TEXTUREWARPEDVIEW

#include <gmGraphics/View.hh>
#include <gmGraphics/TextureInterface.hh>

#include <gmCore/float.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   The TextureWarpedView uses a texture as 0-1 coordinates to warp
   the output from other views.
*/
class TextureWarpedView
  : public View {

public:

  TextureWarpedView();
  virtual ~TextureWarpedView();

  /**
     Appends local renderers to the view settings, renders the views
     and warps the result using the specified texture.
  */
  void renderFullPipeline(ViewSettings settings) override;

  /**
     Adds a view to warp.

     \gmXmlTag{gmGraphics,TextureWarpedView,view}
  */
  void addView(std::shared_ptr<View>);

  /**
     Sets the Texture to use for warping coordinates.

     \gmXmlTag{gmGraphics,TextureWarpedView,texture}
  */
  void setTexture(std::shared_ptr<TextureInterface> tex);

  /**
     De/activates edge-to-edge mode. If set to true, the middle of the
     border texel values of the texture correspond to the location of
     the edge of the pixel buffer to render to, and a color value of
     (0,0) reads off the *center* of the first pixel on the buffer to
     warp. Default is false, meaning that standard OpenGL texture
     coordinate behavior is used, which probably is what you want if
     your warp texture has the same resolution as the render target
     buffer.

     \gmXmlTag{gmGraphics,TextureWarpedView,edgeToEdge}
  */
  void setEdgeToEdge(bool on);

  /**
     Sets a scaling to the texture values to reach outside of the
     texture. Default is [ 1, 1 ].

     \gmXmlTag{gmGraphics,TextureWarpedView,warpScale}
  */
  void setWarpScale(Eigen::Vector2f scale);

  /**
     Sets an offset to add to the texture values, to reach outside of the
     texture. Default is [ 0, 0 ].

     \gmXmlTag{gmGraphics,TextureWarpedView,warpOffset}
  */
  void setWarpOffset(Eigen::Vector2f offset);

  /**
     Removes all renderers and, if recursive is set to true, also
     renderers added to sub dispatchers.
  */
  virtual void clearRenderers(bool recursive = false) override;

  GM_OFI_DECLARE;

private:

  struct Impl;
  std::unique_ptr<Impl> _impl;
};

END_NAMESPACE_GMGRAPHICS;

#endif
