
#ifndef GRAMODS_TRACK_TRACKERBASE
#define GRAMODS_TRACK_TRACKERBASE

#include <gmTrack/config.hh>

#include <gmCore/Object.hh>

BEGIN_NAMESPACE_GMTRACK;

/**
   The base of trackers
 */
class TrackerBase
  : public gmCore::Object {

public:

  struct PoseSample {
    Vector3f position;
    Quaternion orientation;
    std::chrono::time_point time;
  };

  bool getPose(std::vector<PoseSample> &p);

};

END_NAMESPACE_GMTRACK;
