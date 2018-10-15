
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
  void setVelocity(gmTypes::float3 vel);

  /**
     Sets the rotational velocity of the viewpoint as a quaternion,
     defining the rotation per second, in format (w x y z).
  */
  void setQuaternionVelocity(gmTypes::float4 rot);

  /**
     Sets the rotational velocity of the viewpoint as a axis angle
     rotation in format (x y z a) where angle a is expressed in
     radians per second. The axis is normalized during the call.
  */
  void setAxisAngleVelocity(gmTypes::float4 rot);

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
