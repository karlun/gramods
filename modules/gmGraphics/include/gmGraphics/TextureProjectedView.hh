
#ifndef GRAMODS_GRAPHICS_TEXTUREPROJECTEDVIEW
#define GRAMODS_GRAPHICS_TEXTUREPROJECTEDVIEW

// Required before gmCore/OFactory.hh for some compilers
#include <gmCore/io_eigen.hh>
#include <gmCore/io_size.hh>

#include <gmGraphics/MultiscopicView.hh>
#include <gmGraphics/TextureInterface.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   The TextureProjectedView uses a texture to define the 3D position
   of each render target buffer pixel, and then projects the specified
   renderers onto this "surface", from the viewpoint. This is useful
   for rendering a dynamic view on a static projection surface.
*/
class TextureProjectedView
  : public MultiscopicView {

public:

  TextureProjectedView();
  virtual ~TextureProjectedView();

  /**
     Sets the resolution of the intermediate buffer, in
     pixels. Default is 2048 x 2048.

     \gmXmlTag{gmGraphics,TextureProjectedView,bufferResolution}
  */
  void setBufferResolution(gmCore::size2 R);

  /**
     Turn on or off linear interpolation in the projection from linear
     graphics to non-linear projection space. Default is off.

     \gmXmlTag{gmGraphics,TextureProjectedView,linearInterpolation}
  */
  void setLinearInterpolation(bool on);

  /**
     Appends local renderers to the view settings, renders the views
     and projects the result onto the texture's "surface", from the
     viewpoint.
  */
  void renderFullPipeline(ViewSettings settings, Eye eye) override;

  /**
     Propagates the specified visitor.

     @see Object::Visitor
  */
  void traverse(Visitor *visitor) override;

  /**
     Sets the Texture to use for warping coordinates.

     \gmXmlTag{gmGraphics,TextureProjectedView,texture}
  */
  void setTexture(std::shared_ptr<TextureInterface> tex);

  /**
     De/activates edge-to-edge mode. If set to true, the middle of the
     border texel values of the texture correspond to the location of
     the edge of the pixel buffer to render to. Default is false,
     meaning that standard OpenGL texture coordinate behavior is used,
     which probably is what you want if your texture has the same
     resolution as the render target buffer.

     \gmXmlTag{gmGraphics,TextureProjectedView,edgeToEdge}
  */
  void setEdgeToEdge(bool on);

  /**
     Sets a scaling to the texture values, so that the 3D position is
     calculated by p' = p * scale + offset. Default is [ 1, 1, 1 ].

     \gmXmlTag{gmGraphics,TextureProjectedView,warpScale}
  */
  void setWarpScale(Eigen::Vector3f scale);

  /**
     Sets an offset to add to the texture values, so that the 3D
     position is calculated by p' = p * scale + offset. Default is
     [ 0, 0, 0 ].

     \gmXmlTag{gmGraphics,TextureProjectedView,warpOffset}
  */
  void setWarpOffset(Eigen::Vector3f offset);

  GM_OFI_DECLARE;

private:

  struct Impl;
  std::unique_ptr<Impl> _impl;
};

END_NAMESPACE_GMGRAPHICS;

#endif
