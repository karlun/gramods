
#ifndef GM_TRACK_BASE_ESTIMATOR
#define GM_TRACK_BASE_ESTIMATOR

#include <gmTrack/SinglePoseTracker.hh>


class BaseEstimator {
public:

  BaseEstimator();

  void process();

  void setScale(bool on) { scale = on; }

  void setUniform(bool on) { uniform = on; }

  void setTracker(std::shared_ptr<gramods::gmTrack::SinglePoseTracker> tracker) {
    this->tracker = tracker;
  }

private:

  bool scale;
  bool uniform;
  std::shared_ptr<gramods::gmTrack::SinglePoseTracker> tracker;

};

#endif
