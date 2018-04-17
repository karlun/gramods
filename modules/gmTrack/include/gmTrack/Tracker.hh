
#ifndef GRAMODS_TRACK_TRACKER
#define GRAMODS_TRACK_TRACKER

#include <gmTrack/config.hh>

#include <gmCore/Object.hh>
#include <Eigen/Eigen>
#include <chrono>

BEGIN_NAMESPACE_GMTRACK;

/**
   The base of Trackers
 */
class Tracker
  : public gmCore::Object {

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
