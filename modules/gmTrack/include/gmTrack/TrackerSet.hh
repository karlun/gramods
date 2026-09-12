
#ifndef GRAMODS_TRACK_TRACKERSET
#define GRAMODS_TRACK_TRACKERSET

#include <gmTrack/config.hh>
#include <gmTrack/TrackerBase.hh>

#include <gmCore/Object.hh>
#include <gmCore/OFactory.hh>

BEGIN_NAMESPACE_GMTRACK;

/**
   A TrackerSet is a combination of one or more trackers.

   This class does not do anything but group trackers together and
   forward call to them.

   Observe that a stationary TrackerSet may not have a pose tracker
   associated with it and calling getPose would then return
   false. Likewise one or more of the other trackers may also be
   missing in a valid configuration.
*/
class TrackerSet : public gmCore::Object {

public:

  /**
     @see PoseTracker::get
  */
  std::optional<PoseTracker::State> getPose();

  /**
     @see BinaryTracker::get
  */
  std::optional<BinaryTracker::State> getBinary();

  /**
     @see FloatTracker::get
  */
  std::optional<FloatTracker::State> getFloat();

  /**
     @see Float2Tracker::get
  */
  std::optional<Float2Tracker::State> getFloat2();

  /**
     Adds a pose tracker associated with this TrackerSet.

     \gmXmlTag{gmTrack,TrackerSet,poseTracker}
  */
  void addPoseTracker(std::shared_ptr<PoseTracker> t);

  /**
     Adds a binary (bool) tracker associated with this TrackerSet.

     \gmXmlTag{gmTrack,TrackerSet,binaryTracker}
  */
  void addBinaryTracker(std::shared_ptr<BinaryTracker> t);

  /**
     Adds a float tracker associated with this TrackerSet.

     \gmXmlTag{gmTrack,TrackerSet,floatTracker}
  */
  void addFloatTracker(std::shared_ptr<FloatTracker> t);

  /**
     Adds a float2 tracker associated with this TrackerSet.

     \gmXmlTag{gmTrack,TrackerSet,float2Tracker}
  */
  void addFloat2Tracker(std::shared_ptr<Float2Tracker> t);

  /**
     Returns the default key, in Configuration, for the
     Object.
  */
  virtual std::string getDefaultKey() override { return "trackerSet"; }

  /**
     Propagates the specified visitor.

     @see Object::Visitor
  */
  void traverse(Visitor *visitor) override;

  GM_OFI_DECLARE;

private:
  std::vector<std::shared_ptr<PoseTracker>> pose_tracker;
  std::vector<std::shared_ptr<BinaryTracker>> binary_tracker;
  std::vector<std::shared_ptr<FloatTracker>> float_tracker;
  std::vector<std::shared_ptr<Float2Tracker>> float2_tracker;
};

END_NAMESPACE_GMTRACK;

#endif
