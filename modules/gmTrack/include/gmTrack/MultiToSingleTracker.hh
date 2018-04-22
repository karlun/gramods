
#ifndef GRAMODS_TRACK_MULTITOSINGLETRACKER
#define GRAMODS_TRACK_MULTITOSINGLETRACKER

#include <gmTrack/SingleTracker.hh>
#include <gmTrack/MultiTracker.hh>

#include <gmCore/OFactory.hh>

BEGIN_NAMESPACE_GMTRACK;

/**
   The base of MultiToSingleTrackers
 */
class MultiToSingleTracker
  : public SingleTracker {

public:

  /**
     Sets the MultiTracker to extract a single sensor data from.
  */
  void setTracker(std::shared_ptr<MultiTracker> tracker) {
    this->tracker = tracker;
  }

  /**
     Sets the index from the MultiTracker to extract as SingleTracker.
  */
  void setSensor(int idx) {
    sensor_idx = idx;
  }

  /**
     Replaces the contents of p with pose data.
  */
  bool getPose(PoseSample &p);

  GM_OFI_DECLARE(MultiToSingleTracker);

private:

  std::shared_ptr<MultiTracker> tracker;
  int sensor_idx;

};

END_NAMESPACE_GMTRACK;

#endif
