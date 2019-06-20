
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
     Sets the rotational velocity of the viewpoint, in rotation per
     second.
  */
  void setRotation(Eigen::Quaternionf q);

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
