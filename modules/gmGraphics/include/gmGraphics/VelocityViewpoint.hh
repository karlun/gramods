
#ifndef GRAMODS_GRAPHICS_VELOCITYVIEWPOINT
#define GRAMODS_GRAPHICS_VELOCITYVIEWPOINT

#include <gmGraphics/Viewpoint.hh>
#include <gmCore/Updateable.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   The VelocityViewpoint animates the viewpoint position and
   orientation based on specified linear and angular velocities,
   respectively.
*/
class VelocityViewpoint
  : public gmGraphics::Viewpoint,
    public gmCore::Updateable {

public:

  VelocityViewpoint();

  /**
     Sets the linear (position) velocity of the viewpoint, in meters
     per second.
  */
  void setVelocity(Eigen::Vector3f vel);

  /**
     Sets the rotational velocity of the viewpoint as a quaternion,
     defining the rotation per second, in xml as (w x y z).
  */
  void setQuaternionVelocity(Eigen::Quaternionf q);

  /**
     Sets the rotational velocity of the viewpoint as a axis angle
     rotation, in xml as (a x y z) where angle a is expressed in
     radians per second. The axis is automatically normalized.
  */
  void setAngleAxisVelocity(Eigen::AngleAxisf aa);

  /**
     Updates the animation.
  */
  void update(clock::time_point t);

  GM_OFI_DECLARE;

private:

  struct Impl;
  std::unique_ptr<Impl> _impl;

};

END_NAMESPACE_GMGRAPHICS;

#endif
