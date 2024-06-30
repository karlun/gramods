
#ifndef GRAMODS_TRACK_MULTITOSINGLEPOSETRACKER
#define GRAMODS_TRACK_MULTITOSINGLEPOSETRACKER

#include <gmTrack/SinglePoseTracker.hh>
#include <gmTrack/MultiPoseTracker.hh>

#include <gmCore/OFactory.hh>

BEGIN_NAMESPACE_GMTRACK;

/**
   The base of MultiToSinglePoseTrackers
 */
class MultiToSinglePoseTracker
  : public SinglePoseTracker {

public:

  /**
     Sets the MultiPoseTracker to extract a single sensor data from.
  */
  void setMultiPoseTracker(std::shared_ptr<MultiPoseTracker> tracker) {
    this->tracker = tracker;
  }

  /**
     Sets the index from the MultiPoseTracker to extract as SinglePoseTracker.

     \gmXmlTag{gmTrack,MultiToSinglePoseTracker,sensor}
  */
  void setSensor(int idx) {
    sensor_idx = idx;
  }

  /**
     Replaces the contents of p with pose data.
  */
  bool getPose(PoseSample &p) override;

  /**
     Propagates the specified visitor.

     @see Object::Visitor
  */
  void traverse(Visitor *visitor) override;

  GM_OFI_DECLARE;

private:

  std::shared_ptr<MultiPoseTracker> tracker;
  int sensor_idx;

};

END_NAMESPACE_GMTRACK;

#endif
