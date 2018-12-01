
#ifndef GM_TRACK_BASE_ESTIMATOR
#define GM_TRACK_BASE_ESTIMATOR

#include <gmTrack/config.hh>

#include <gmTypes/all.hh>
#include <gmCore/Object.hh>
#include <gmTrack/Controller.hh>
#include <gmCore/Updateable.hh>

BEGIN_NAMESPACE_GMTRACK;

/**
   The BaseEstimator is a utility for estimating the base of a
   tracking system's coordinates, i e registration of the tracking
   system into room or display system coordinates.

   The utility is used by marking, with the tracking device, the
   position of known, pre-specified positions in the room or display
   system.
*/
class BaseEstimator
  : public gmCore::Object,
    public gmCore::Updateable {

public:

  BaseEstimator();

  /**
     Updates the animation.
  */
  void update(clock::time_point t);

  /**
     Sets the controller to use for calibration.
  */
  void setController(std::shared_ptr<Controller> controller);

  /**
     Adds a known calibration point, in room coordinates.
  */
  void addPoint(gmTypes::float3 p);

  /**
     Sets the frequency at which samples are collected when the
     controller button is pressed. Default is 1. At most one sample
     per frame will be used regardless of this value.
  */
  void setSamplesPerSecond(float n);

  /**
     Extract registration matrix, either raw or without
     scaling. Returns true if there is a registration, false
     otherwise.
  */
  bool getRegistration(Eigen::Matrix4f * RAW, Eigen::Matrix4f * UNIT);

  GM_OFI_DECLARE;

private:

  struct Impl;
  std::unique_ptr<Impl> _impl;

};

END_NAMESPACE_GMTRACK;

#endif
