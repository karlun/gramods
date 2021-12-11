
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
     Returns the default key, in Configuration, for the
     Object. Returns singlePoseTracker.
  */
  virtual std::string getDefaultKey() override { return "singlePoseTracker"; }

  /**
     Replaces the contents of p with pose data. Returns true if data
     could be read, false otherwise. Use sample time to check if data
     are fresh.
  */
  virtual bool getPose(PoseSample &p) = 0;

};

END_NAMESPACE_GMTRACK;

#endif
