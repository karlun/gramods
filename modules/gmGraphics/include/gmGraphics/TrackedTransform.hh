
#ifndef GRAMODS_GRAPHICS_TRACKEDTRANSFORM
#define GRAMODS_GRAPHICS_TRACKEDTRANSFORM

#include <gmGraphics/Group.hh>

#include <gmTrack/SinglePoseTracker.hh>
#include <gmTrack/MultiPoseTracker.hh>

BEGIN_NAMESPACE_GMGRAPHICS;

/**
   The scenegraph TrackedTransform base.
*/
class TrackedTransform : public Group {

public:
  TrackedTransform();
  ~TrackedTransform();

  /**
     Sets the time, in seconds, a tracker lingers after ceasing to
     update their data, before it's removed. Default is 5 seconds.
  */
  void setHysteresis(float t);

  /**
     Adds a tracker to read pose data from.
  */
  void addSinglePoseTracker(std::shared_ptr<gmTrack::SinglePoseTracker> t);

  /**
     Adds a tracker to read pose data from.
  */
  void addMultiPoseTracker(std::shared_ptr<gmTrack::MultiPoseTracker> t);

  void accept(Visitor *visitor) override;

  GM_OFI_DECLARE;

private:
  struct Impl;
  std::unique_ptr<Impl> _impl;
};

END_NAMESPACE_GMGRAPHICS;

#endif
