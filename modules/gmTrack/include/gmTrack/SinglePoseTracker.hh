
#ifndef GRAMODS_TRACK_SINGLEPOSETRACKER
#define GRAMODS_TRACK_SINGLEPOSETRACKER

#include <gmTrack/PoseTracker.hh>

BEGIN_NAMESPACE_GMTRACK;

/**
   The base of SinglePoseTrackers
 */
class SinglePoseTracker
  : public PoseTracker {

public:

  /**
     Replaces the contents of p with pose data.
  */
  virtual bool getPose(PoseSample &p) = 0;

};

END_NAMESPACE_GMTRACK;

#endif