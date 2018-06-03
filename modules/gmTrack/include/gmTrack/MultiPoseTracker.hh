
#ifndef GRAMODS_TRACK_MULTIPOSETRACKER
#define GRAMODS_TRACK_MULTIPOSETRACKER

#include <gmTrack/PoseTracker.hh>

#include <map>

BEGIN_NAMESPACE_GMTRACK;

/**
   The base of MultiPoseTrackers
 */
class MultiPoseTracker
  : public PoseTracker {

public:

  /**
     Replaces the contents of p with pose data.
  */
  virtual bool getPose(std::map<int, PoseSample> &p) = 0;

};

END_NAMESPACE_GMTRACK;

#endif
