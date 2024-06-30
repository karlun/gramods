
#ifndef GRAMODS_GRAPHICS_POSEDSPHERICALVIEW
#define GRAMODS_GRAPHICS_POSEDSPHERICALVIEW

#include <gmGraphics/View.hh>
#include <gmGraphics/CoordinatesMapper.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   This view provides a symmetrical projection of the renderers, on a
   spherical projection surface.

   The PosedSphericalView maps the renderers' graphics into a
   spherical projection surface, centered on the viewpoint's position,
   from the viewpoint's position and orientation.
*/
class PosedSphericalView
  : public View {

public:

  PosedSphericalView();

  /**
     Dispatches renderers.
  */
  void renderFullPipeline(ViewSettings settings) override;

  /**
     Propagates the specified visitor.

     @see Object::Visitor
  */
  void traverse(Visitor *visitor) override;

  /**
     Sets the resolution of the intermediate cube map, in
     pixels. Default is 2048.

     \gmXmlTag{gmGraphics,PosedSphericalView,cubeMapResolution}
  */
  void setCubeMapResolution(int res);

  /**
     Turn on or off linear interpolation in the projection from linear
     graphics to non-linear projection space. Default is off.

     \gmXmlTag{gmGraphics,PosedSphericalView,linearInterpolation}
  */
  void setLinearInterpolation(bool on);

  /**
     Sets the coordinates mapper that should be used to map the 3D
     surrounding into the 2D view.

     \gmXmlTag{gmGraphics,PosedSphericalView,coordinatesMapper}
  */
  void setCoordinatesMapper(std::shared_ptr<CoordinatesMapper> mapper);

  /**
     When set to true, the output on the view will be square, so that
     using a fisheye mapper would render the view as a circle instead
     of an ellipse. Default is false.
  */
  void setMakeSquare(bool on);

  GM_OFI_DECLARE;

private:

  struct Impl;
  std::unique_ptr<Impl> _impl;

};

END_NAMESPACE_GMGRAPHICS;

#endif
