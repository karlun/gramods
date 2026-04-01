
#ifndef GRAMODS_GRAPHICS_TRACKEDVIEWPOINT
#define GRAMODS_GRAPHICS_TRACKEDVIEWPOINT

#include <gmGraphics/Viewpoint.hh>

#include <gmTrack/TrackerBase.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   The TrackedViewpoint is a Viewpoint that uses a PoseTracker
   to updates its pose.
*/
class TrackedViewpoint : public gmGraphics::Viewpoint {

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
     Sets the key to use from the tracker. If not set
     gmTrack::StdKey::HEAD is used.

     \gmXmlTag{gmGraphics,TrackedViewpoint,key}
  */
  void setKey(std::string key);

  /**
     Sets the tracker to read pose data from.

     \gmXmlTag{gmGraphics,TrackedViewpoint,poseTracker}
  */
  void setPoseTracker(std::shared_ptr<gmTrack::PoseTracker> t);

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
