
#ifndef GRAMODS_GRAPHICS_GEOMETRYCORRECTEDPROJECTORVIEW
#define GRAMODS_GRAPHICS_GEOMETRYCORRECTEDPROJECTORVIEW

// Required before gmCore/OFactory.hh for some compilers
#include <gmCore/io_float.hh>
#include <gmCore/io_eigen.hh>
#include <gmCore/io_angle.hh>

#include <gmGraphics/MultiscopicView.hh>

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
  : public MultiscopicView {

public:

  GeometryCorrectedProjectorView();
  virtual ~GeometryCorrectedProjectorView();

  /**
     Propagates the specified visitor.

     @see Object::Visitor
  */
  void traverse(Visitor *visitor) override;

  /**
     Sets the resolution of the intermediate buffer, in
     pixels. Default is 2048.

     \gmXmlTag{gmGraphics,GeometryCorrectedProjectorView,bufferWidth}
  */
  void setBufferWidth(int W);

  /**
     Sets the resolution of the intermediate buffer, in
     pixels. Default is 2048.

     \gmXmlTag{gmGraphics,GeometryCorrectedProjectorView,bufferHeight}
  */
  void setBufferHeight(int H);

  /**
     Turn on or off linear interpolation in the projection from linear
     graphics to non-linear projection space. Default is off.

     \gmXmlTag{gmGraphics,GeometryCorrectedProjectorView,linearInterpolation}
  */
  void setLinearInterpolation(bool on);

  /**
     Set the top left corner of the projection.

     \gmXmlTag{gmGraphics,GeometryCorrectedProjectorView,topLeftCorner}

     \sa gramods::operator>>(std::istream &, Eigen::Vector3f &)
  */
  void setTopLeftCorner(Eigen::Vector3f tlc);

  /**
     Set the bottom right corner of the projection.

     \gmXmlTag{gmGraphics,GeometryCorrectedProjectorView,bottomRightCorner}

     \sa gramods::operator>>(std::istream &, Eigen::Vector3f &)
  */
  void setBottomRightCorner(Eigen::Vector3f brc);

  /**
     Set the position of the optic center of the projection.

     \gmXmlTag{gmGraphics,GeometryCorrectedProjectorView,position}

     \sa gramods::operator>>(std::istream &, Eigen::Vector3f &)
  */
  void setPosition(Eigen::Vector3f p);

  /**
     Set the position and orientation of the projection as an
     extrinsics matrix (12 values, 4 column 3-vectors, row major):

     \gmXmlTag{gmGraphics,GeometryCorrectedProjectorView,extrinsics}
  */
  void setExtrinsics(gmCore::float12 M);

  /**
     Set the projection shape, as intrinsics (fx fy cx cy), in
     unitless values assuming a sensor size of 1x1.

     \gmXmlTag{gmGraphics,GeometryCorrectedProjectorView,intrinsics}
  */
  void setIntrinsics(gmCore::float4 m);

  /**
     Angles between forward direction and the clip planes left, right,
     bottom and top, in radians. Left and bottom angles increase
     left-wise and downwards, respectively, while right and top angles
     increase right-wise and upwards, respectively.

     \gmXmlTag{gmGraphics,GeometryCorrectedProjectorView,clipAngles}

     \sa operator>>(std::istream &, gmCore::angle &)
  */
  void setClipAngles(gmCore::angle4 a);

  /**
     Set orientation of the projection, of the extrinsics.

     \gmXmlTag{gmGraphics,GeometryCorrectedProjectorView,orientation}

     \sa gramods::operator>>(std::istream &, Eigen::Quaternionf &)
  */
  void setOrientation(Eigen::Quaternionf q);

  /**
     Set the geometry to use for projection correction.

     \gmXmlTag{gmGraphics,GeometryCorrectedProjectorView,geometry}
  */
  void setGeometry(std::shared_ptr<Geometry> g);

  GM_OFI_DECLARE;

protected:

  /**
     Renders one eye at a time, or just a mono eye.
  */
  void renderFullPipeline(ViewSettings settings, Eye eye) override;

private:

  struct Impl;
  std::unique_ptr<Impl> _impl;
};

END_NAMESPACE_GMGRAPHICS;

#endif
