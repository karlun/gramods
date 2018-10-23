
#ifndef GRAMODS_TRACK_POSETRACKER
#define GRAMODS_TRACK_POSETRACKER

#include <gmTrack/config.hh>

#include <gmCore/Object.hh>
#include <Eigen/Eigen>
#include <chrono>

BEGIN_NAMESPACE_GMTRACK;

/**
   The base of PoseTrackers
 */
class PoseTracker
  : public gmCore::Object {

  /**
     Returns the default key, in Configuration, for the
     Object.
  */
  virtual std::string getDefaultKey() { return "tracker"; }

public:

  typedef std::chrono::steady_clock clock;

  struct PoseSample {
    Eigen::Vector3f position;
    Eigen::Quaternionf orientation;
    clock::time_point time;
  };
};

END_NAMESPACE_GMTRACK;

#endif
