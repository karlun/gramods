
#ifndef GRAMODS_TRACK_SINGLETOMULTIPOSETRACKER
#define GRAMODS_TRACK_SINGLETOMULTIPOSETRACKER

#include <gmTrack/SinglePoseTracker.hh>
#include <gmTrack/MultiPoseTracker.hh>

#include <gmCore/OFactory.hh>

BEGIN_NAMESPACE_GMTRACK;

/**
   The base of SingleToMultiPoseTrackers
 */
class SingleToMultiPoseTracker
  : public MultiPoseTracker {

public:

  /**
     Adds the SinglePoseTracker to extract a sensor data from.
  */
  void addSinglePoseTracker(std::shared_ptr<SinglePoseTracker> tracker) {
    trackers.push_back(tracker);
  }

  /**
     Adds or updates pose data in the specified map. Returns true if
     data could be read, false otherwise. Use sample time to check if
     data of a specific sensor are fresh.
  */
  bool getPose(std::map<int, PoseSample> &p) override;

  /**
     Propagates the specified visitor.

     @see Object::Visitor
  */
  void traverse(Visitor *visitor) override;

  GM_OFI_DECLARE;

private:

  std::vector<std::shared_ptr<SinglePoseTracker>> trackers;

};

END_NAMESPACE_GMTRACK;

#endif
