
#ifndef GRAMODS_TRACK_MULTITRACKER
#define GRAMODS_TRACK_MULTITRACKER

#include <gmTrack/Tracker.hh>

#include <map>

BEGIN_NAMESPACE_GMTRACK;

/**
   The base of MultiTrackers
 */
class MultiTracker
  : public Tracker {

public:

  /**
     Replaces the contents of p with pose data.
  */
  virtual bool getPose(std::map<int, PoseSample> &p) = 0;

};

END_NAMESPACE_GMTRACK;

#endif
