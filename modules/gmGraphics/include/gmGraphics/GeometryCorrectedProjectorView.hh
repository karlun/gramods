
#ifndef GRAMODS_GRAPHICS_GEOMETRYCORRECTEDPROJECTORVIEW
#define GRAMODS_GRAPHICS_GEOMETRYCORRECTEDPROJECTORVIEW

#include <gmGraphics/StereoscopicView.hh>

#include <gmGraphics/Geometry.hh>

#include <gmTypes/float.hh>
#include <gmTypes/eigen.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   Geometrically corrected projector view, a view that is undistorted
   by geometrical projection.

   Use this view if there is a projector that shows its image on a
   geometrically non-complex surface (such as a flat, cylindrical or
   spherical surface) and the observer, or viewer, is not co-located
   with this projector but still wants a non-distorted view.

   To function correctly, this view needs to be configured with three
   settings: position, orientation and frustum shape.

   The estimation of the frustum shape from specified corners will
   *not* control the orientation of the projection, but rather take
   orientation into account. This means that the corners positions of
   the projection can be measured directly on the (potentially
   non-linear) projection surface.
*/
class GeometryCorrectedProjectorView
  : public StereoscopicView {

public:

  GeometryCorrectedProjectorView();
  virtual ~GeometryCorrectedProjectorView();

  /**
     Sets the resolution of the intermediate buffer, in
     pixels. Default is 2048.

     \b XML-attribute: \c bufferWidth
  */
  void setBufferWidth(int W);

  /**
     Sets the resolution of the intermediate buffer, in
     pixels. Default is 2048.

     \b XML-attribute: \c bufferHeight
  */
  void setBufferHeight(int H);

  /**
     Turn on or off linear interpolation in the projection from linear
     graphics to non-linear projection space. Default is off.

     \b XML-attribute: \c linearInterpolation
  */
  void setLinearInterpolation(bool on);

  /**
     Set the top left corner of the projection.

     \b XML-attribute: \c topLeftCorner
  */
  void setTopLeftCorner(Eigen::Vector3f tlc);

  /**
     Set the bottom right corner of the projection.

     \b XML-attribute: \c bottomRightCorner
  */
  void setBottomRightCorner(Eigen::Vector3f brc);

  /**
     Set the position of the optic center of the projection.

     \b XML-attribute: \c position
  */
  void setPosition(Eigen::Vector3f p);

  /**
     Set the position and orientation of the projection as an
     extrinsics matrix (12 values, 4 column 3-vectors, row major):

     \b XML-attribute: \c extrinsics
  */
  void setExtrinsics(gmTypes::float12 M);

  /**
     Set the projection shape, as intrinsics (fx fy cx cy), in
     unitless values assuming a sensor size of 1x1.

     \b XML-attribute: \c intrinsics
  */
  void setIntrinsics(gmTypes::float4 m);

  /**
     Set the projection shape, as left, right, bottom, top clip plane
     angles, in radians.

     \b XML-attribute: \c clipAngles
  */
  void setClipAngles(gmTypes::float4 a);

  /**
     Set orientation of the projection, of the extrinsics.

     \b XML-attribute: \c orientation
  */
  void setOrientation(Eigen::Quaternionf q);

  /**
     Set the geometry to use for projection correction.

     \b XML-key: \c geometry
  */
  void setGeometry(std::shared_ptr<Geometry> g);

  GM_OFI_DECLARE;

protected:

  /**
     Renders one eye at a time, or just a mono eye.
  */
  void renderFullPipeline(ViewSettings settings, Eye eye);

private:

  struct Impl;
  std::unique_ptr<Impl> _impl;
};

END_NAMESPACE_GMGRAPHICS;

#endif
