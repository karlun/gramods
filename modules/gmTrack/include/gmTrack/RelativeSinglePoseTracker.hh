
#ifndef GRAMODS_TRACK_RELATIVESINGLEPOSETRACKER
#define GRAMODS_TRACK_RELATIVESINGLEPOSETRACKER

#include <gmTrack/SinglePoseTracker.hh>

#include <gmCore/OFactory.hh>

BEGIN_NAMESPACE_GMTRACK;

/**
   The base of RelativeSinglePoseTrackers
 */
class RelativeSinglePoseTracker
  : public SinglePoseTracker {

public:

  /**
     Replaces the contents of p with pose data.
  */
  bool getPose(PoseSample &p) override;

  /**
     Sets the tracker to be used to read off the origin pose of the
     target tracker.

     \gmXmlTag{gmTrack,RelativeSinglePoseTracker,originTracker}
   */
  void setOriginTracker(std::shared_ptr<SinglePoseTracker> tracker);

  /**
     Sets the tracker to be used to read off the target pose relative
     to the pose extracted from the origin tracker.

     \gmXmlTag{gmTrack,RelativeSinglePoseTracker,targetTracker}
  */
  void setTargetTracker(std::shared_ptr<SinglePoseTracker> tracker);

  /**
     Propagates the specified visitor.

     @see Object::Visitor
  */
  void traverse(Visitor *visitor) override;

  GM_OFI_DECLARE;
  
private:

  std::shared_ptr<SinglePoseTracker> origin_tracker;

  std::shared_ptr<SinglePoseTracker> target_tracker;
};

END_NAMESPACE_GMTRACK;

#endif
