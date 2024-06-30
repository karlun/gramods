
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
  Eigen::Vector3f getPosition(Eye eye = Eye::MONO);

  /**
     Returns the orientation of the TrackedViewpoint.
  */
  Eigen::Quaternionf getOrientation(Eye eye = Eye::MONO);

  /**
     Sets the tracker to read pose data from.
  */
  void setSinglePoseTracker(std::shared_ptr<gmTrack::SinglePoseTracker> t);

  /**
     Propagates the specified visitor.

     @see Object::Visitor
  */
  void traverse(Visitor *visitor) override;

  GM_OFI_DECLARE;

protected:

  struct Impl;
  std::unique_ptr<Impl> _impl;

};

END_NAMESPACE_GMGRAPHICS;

#endif
