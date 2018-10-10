
#ifndef GRAMODS_GRAPHICS_TRACKEDVIEWPOINT
#define GRAMODS_GRAPHICS_TRACKEDVIEWPOINT

#include <gmGraphics/Viewpoint.hh>

#include <gmTrack/SinglePoseTracker.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   The TrackedViewpoint is a Viewpoint that uses a SinglePoseTracker
   to updates its pose.
*/
class TrackedViewpoint
  : public gmGraphics::Viewpoint {

public:

  TrackedViewpoint();

  /**
     Returns the position of the Viewpoint.
  */
  Eigen::Vector3f getPosition();

  /**
     Returns the orientation of the TrackedViewpoint.
  */
  Eigen::Quaternionf getOrientation();

  /**
     Sets the tracker to read pose data from.
  */
  void setTracker(std::shared_ptr<gmTrack::SinglePoseTracker> t);

  GM_OFI_DECLARE;

protected:

  class Impl;
  std::unique_ptr<Impl> _impl;

};

END_NAMESPACE_GMGRAPHICS;

#endif