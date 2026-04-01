
#ifndef GRAMODS_TRACK_RELATIVEPOSETRACKER
#define GRAMODS_TRACK_RELATIVEPOSETRACKER

#include <gmTrack/TrackerBase.hh>

#include <gmCore/OFactory.hh>

BEGIN_NAMESPACE_GMTRACK;

/**
   This decorator will change the origin of a selected set or all of
   the poses of a specified tracker.
 */
class RelativePoseTracker : public PoseTracker {

public:

  RelativePoseTracker();
  virtual ~RelativePoseTracker();

  /**
     @see TrackerBase::get
  */
  std::optional<State> get() override;

  /**
     Sets the key in the origin tracker associated with the pose to
     use as origin for the relative pose states. If this is not set,
     then each key in the tracker that also has a corresponding key in
     the origin tracker will have its pose transformed into the space
     of that corresponding pose. Other states are untouched.

     \gmXmlTag{gmTrack,RelativePoseTracker,originKey}
  */
  void setOriginKey(std::string key);

  /**
     Sets the tracker to be used to read off the origin pose. This is
     only needed if the origin is read off a different tracker than
     the states that are transformed.

     \gmXmlTag{gmTrack,RelativePoseTracker,originTracker}
  */
  void setOriginTracker(std::shared_ptr<PoseTracker> tracker);

  /**
     Adds a pose to transform into an origin's space.

     \gmXmlTag{gmTrack,RelativePoseTracker,relativeKey}
   */
  void addRelativeKey(std::string key);

  /**
     Sets the tracker to be transformed into another origin. If
     another origin tracker is not set, then this tracker will also be
     used to extract the origin.

     \gmXmlTag{gmTrack,RelativePoseTracker,poseTracker}
  */
  void setPoseTracker(std::shared_ptr<PoseTracker> tracker);

  /**
     Propagates the specified visitor.

     @see Object::Visitor
  */
  void traverse(Visitor *visitor) override;

  GM_OFI_DECLARE;
  
private:

  struct Impl;
  std::unique_ptr<Impl> _impl;
};

END_NAMESPACE_GMTRACK;

#endif
