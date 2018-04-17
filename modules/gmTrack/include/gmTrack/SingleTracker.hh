
#ifndef GRAMODS_TRACK_SINGLETRACKER
#define GRAMODS_TRACK_SINGLETRACKER

#include <gmTrack/Tracker.hh>

BEGIN_NAMESPACE_GMTRACK;

/**
   The base of SingleTrackers
 */
class SingleTracker
  : public Tracker {

public:

  /**
     Replaces the contents of p with pose data.
  */
  virtual bool getPose(PoseSample &p) = 0;

};

END_NAMESPACE_GMTRACK;
