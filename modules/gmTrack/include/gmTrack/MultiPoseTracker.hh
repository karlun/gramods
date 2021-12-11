
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
     Returns the default key, in Configuration, for the
     Object. Returns multiPoseTracker.
  */
  virtual std::string getDefaultKey() override { return "multiPoseTracker"; }

  /**
     Adds or updates pose data in the specified map. Returns true if
     data could be read, false otherwise. Use sample time to check if
     data of a specific sensor are fresh.
  */
  virtual bool getPose(std::map<int, PoseSample> &p) = 0;

};

END_NAMESPACE_GMTRACK;

#endif
