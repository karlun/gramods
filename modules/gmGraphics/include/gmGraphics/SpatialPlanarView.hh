
#ifndef GRAMODS_GRAPHICS_SPATIALPLANARVIEW
#define GRAMODS_GRAPHICS_SPATIALPLANARVIEW

#include <gmGraphics/MultiscopicView.hh>

#include <gmTypes/float.hh>
#include <gmTypes/eigen.hh>
#include <gmTypes/angle.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   This view provides skewed frustum projection of the renderers.

   The projection plane can be specified in two ways. The primary way
   is to explicitly set the top-left and bottom-right corners and up
   direction. Alternatively the same corner can be calculated from a
   *virtual* frustum, set with position, orientation, clip angles and
   a distance to the projection plane. Observe, however, that the
   actual frustum for the rendering is controlled by the Viewpoint,
   not these parameters.
*/
class SpatialPlanarView
  : public MultiscopicView {

public:

  SpatialPlanarView();
  virtual ~SpatialPlanarView();

  /**
     Position of the top left corner of the view.

     \b XML-attribute: \c topLeftCorner
  */
  void setTopLeftCorner(Eigen::Vector3f tlc);

  /**
     Position of bottom right corner of the view.

     \b XML-attribute: \c bottomRightCorner
  */
  void setBottomRightCorner(Eigen::Vector3f brc);

  /**
     Set up direction of the view.

     \b XML-attribute: \c upDirection
  */
  void setUpDirection(Eigen::Vector3f up);

  /**
     Set the position of a *virtual* frustum. Default is the
     origin. This will help specifying the projection plane, but will
     not control the viewpoint.

     \b XML-attribute: \c position
  */
  void setPosition(Eigen::Vector3f p);

  /**
     Set the clip angles of a *virtual* frustum, as left, right,
     bottom, top. This will help specifying the projection plane,
     together with position and orientation, but will not control the
     final view angle.

     \b XML-attribute: \c clipAngles
  */
  void setClipAngles(gmTypes::angle4 a);

  /**
     Set the orientation of a *virtual* frustum. Default is zero
     rotation. This will help specifying the projection plane, but
     will not control the viewpoint.

     \b XML-attribute: \c orientation
  */
  void setOrientation(Eigen::Quaternionf q);

  /**
     Set the orientation of a *virtual* frustum. This will help
     specifying the projection plane, but will not control the
     viewpoint.

     \b XML-attribute: \c distance
  */
  void setDistance(float d);

  GM_OFI_DECLARE;

private:

  void renderFullPipeline(ViewSettings settings, Eye eye);

  struct Impl;
  std::unique_ptr<Impl> _impl;
};

END_NAMESPACE_GMGRAPHICS;

#endif
