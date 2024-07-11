
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

public:

  typedef std::chrono::steady_clock clock;

  struct PoseSample {
    Eigen::Vector3f position;
    Eigen::Quaternionf orientation;
    clock::time_point time;

    Eigen::Affine3f asMatrix() const {
      return Eigen::Translation3f(position) * orientation;
    }
  };
};

END_NAMESPACE_GMTRACK;

#endif
