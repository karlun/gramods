
#ifndef GRAMODS_GRAPHICS_GEOMETRYCORRECTEDPROJECTORVIEW
#define GRAMODS_GRAPHICS_GEOMETRYCORRECTEDPROJECTORVIEW

#include <gmGraphics/StereoscopicView.hh>

#include <gmGraphics/Geometry.hh>

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
  */
  void setBufferWidth(int W);

  /**
     Sets the resolution of the intermediate buffer, in
     pixels. Default is 2048.
  */
  void setBufferHeight(int H);

  /**
     Set the top left corner of the projection.
  */
  void setTopLeftCorner(gmTypes::float3 tlc);

  /**
     Set the bottom right corner of the projection.
  */
  void setBottomRightCorner(gmTypes::float3 brc);

  /**
     Set the position of the optic center of the projection.
  */
  void setPosition(gmTypes::float3 p);

  /**
     Set the position and orientation of the projection as an
     extrinsics matrix (12 values, 4 column 3-vectors, row major):
  */
  void setExtrinsics(gmTypes::float12 M);

  /**
     Set the projection shape, as intrinsics (fx fy cx cy), in
     unitless values assuming a sensor size of 1x1.
  */
  void setIntrinsics(gmTypes::float4 m);

  /**
     Set the projection shape, as left, right, bottom, top clip plane
     angles, in radians.
  */
  void setClipAngles(gmTypes::float4 a);

  /**
     Set orientation of the projection, of the extrinsics, as
     quaternion in format (w x y z).
  */
  void setQuaternion(gmTypes::float4 q);

  /**
     Set orientation of the projection, of the extrinsics, as an axis
     angle in format (x y z a), where angle a is expressed in radians.
  */
  void setAxisAngle(gmTypes::float4 aa);

  /**
     Set orientation of the projection, of the extrinsics, as euler
     angles, in radians.
  */
  void setEulerAngles(gmTypes::float3 ea);

  /**
     Set the geometry to use for projection correction.
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
