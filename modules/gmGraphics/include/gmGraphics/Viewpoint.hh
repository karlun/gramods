
#ifndef GRAMODS_GRAPHICS_VIEWPOINT
#define GRAMODS_GRAPHICS_VIEWPOINT

#include <gmGraphics/config.hh>

#include <gmTypes/eigen.hh>

#include <gmCore/Object.hh>
#include <gmCore/OFactory.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   The viewpoint represents a point in space from which the view is to
   be rendered, and an orientation. The orientation may be used for
   view orientation or for eye separation. The view may use this
   Viewpoint to create Camera fitting for rendering.

   Sub classes may implement dynamically updated Viewpoints, based on
   animation or tracking data.
*/
class Viewpoint
  : public gmCore::Object {

public:

  /**
     Returns the position of the viewpoint.
  */
  virtual Eigen::Vector3f getPosition() {
    return position;
  }

  /**
     Returns the orientation of the viewpoint. Observe that a view may
     ignore the orientation value if this does not make any difference
     for its appearance.
  */
  virtual Eigen::Quaternionf getOrientation() {
    return orientation;
  }

  /**
     Explicitly sets the position of the Viewpoint. This may be
     ignored or instantaneously overwritten by dynamic updates of the
     position value.

     \b XML-attribute: \c position
  */
  virtual void setPosition(Eigen::Vector3f p) {
    position = p;
  }

  /**
     Explicitly sets the orientation of the Viewpoint.

     This may be ignored or instantaneously overwritten by dynamic
     updates of the position value.

     \b XML-attribute: \c orientation
  */
  virtual void setOrientation(Eigen::Quaternionf q) {
    orientation = q;
  }

  /**
     Set the up direction to be used in a later call to
     setLookAt. This does nothing if setLookAt is not used.

     \b XML-attribute: \c upDirection
  */
  virtual void setUpDirection(Eigen::Vector3f up) {
    up_direction = up.normalized();
  }

  /**
     Implicitly rotates the viewpoint to look at the specified
     point. This makes use of the currently set viewpoint position and
     up direction, so make sure that those are set before this is
     used.

     Observe that this affects only the viewpoint orientation -
     camera orientation will not automatically toe-in to this point.

     \b XML-attribute: \c lookAt
  */
  virtual void setLookAt(Eigen::Vector3f p);

  /**
     Returns the default key, in Configuration, for the
     Object.
  */
  virtual std::string getDefaultKey() { return "viewpoint"; }

  GM_OFI_DECLARE;

protected:

  Eigen::Vector3f position = Eigen::Vector3f::Zero();
  Eigen::Quaternionf orientation = Eigen::Quaternionf::Identity();

  Eigen::Vector3f up_direction = Eigen::Vector3f(0, 1, 0);

};

END_NAMESPACE_GMGRAPHICS;

#endif
