
#ifndef GRAMODS_GRAPHICS_VIEWPOINT
#define GRAMODS_GRAPHICS_VIEWPOINT

#include <gmGraphics/config.hh>

#include <gmTypes/all.hh>
#include <Eigen/Eigen>

#include <gmCore/Object.hh>
#include <gmCore/OFactory.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   The viewpoint represents a point in space from which the view is to
   be rendered, and an orientation to use for eye separation. The view
   may use this Viewpoint to create Camera fitting for rendering.

   Sub classes may implement dynamically updated Viewpoints, based on
   animation or tracking data.
*/
class Viewpoint
  : public gmCore::Object {

public:

  enum struct Eye {
    MONO,
    LEFT,
    RIGHT
  };

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

     Observe that there is no setOrientation - use setQuaternion or
     setAxisAngle.
  */
  virtual Eigen::Quaternionf getOrientation() {
    return orientation;
  }

  /**
     Explicitly sets the position of the Viewpoint. This may be
     ignored or instantaneously overwritten by dynamic updates of the
     position value.
   */
  virtual void setPosition(gmTypes::float3 pos) {
    position = Eigen::Vector3f(pos[0], pos[1], pos[2]);
  }

  /**
     Explicitly sets the orientation of the Viewpoint as a quaternion
     in format (w x y z).

     This may be ignored or instantaneously overwritten by dynamic
     updates of the position value.
   */
  virtual void setQuaternion(gmTypes::float4 rot) {
    orientation = Eigen::Quaternionf(rot[0], rot[1], rot[2], rot[3]);
  }

  /**
     Explicitly sets the orientation of the Viewpoint as a axis angle
     rotation in format (x y z a) where angle a is expressed in
     radians. The axis is normalized during the call.

     This may be ignored or instantaneously overwritten by dynamic
     updates of the position value.
   */
  virtual void setAxisAngle(gmTypes::float4 rot) {
    orientation = Eigen::Quaternionf
      (Eigen::Quaternionf::AngleAxisType(rot[3], Eigen::Vector3f(rot[0], rot[1], rot[2]).normalized()));
  }

  GM_OFI_DECLARE;

protected:

  Eigen::Vector3f position;
  Eigen::Quaternionf orientation;
};

END_NAMESPACE_GMGRAPHICS;

#endif
