
#ifndef GM_TRACK_POSEREGISTRATIONESTIMATOR
#define GM_TRACK_POSEREGISTRATIONESTIMATOR

#include <gmTrack/config.hh>

#include <gmCore/eigen.hh>
#include <gmCore/Object.hh>
#include <gmTrack/Controller.hh>
#include <gmCore/Updateable.hh>

BEGIN_NAMESPACE_GMTRACK;

/**
   The PoseRegistrationEstimator is a utility for estimating the base
   of a tracking system's coordinates, i e registration of the
   tracking system into room or display system coordinates.

   The utility is used by marking, with the tracking device, the
   position of known, pre-specified positions in the room or display
   system.
*/
class PoseRegistrationEstimator
  : public gmCore::Object,
    public gmCore::Updateable {

public:

  PoseRegistrationEstimator();
  ~PoseRegistrationEstimator();

  /**
     Updates the animation.
  */
  void update(clock::time_point t) override;

  /**
     Sets the controller to use for calibration.

     \gmXmlTag{gmTrack,PoseRegistrationEstimator,controller}
  */
  void setController(std::shared_ptr<Controller> controller);

  /**
     Adds a known calibration point, in room coordinates.

     \gmXmlTag{gmTrack,PoseRegistrationEstimator,point}
  */
  void addPoint(Eigen::Vector3f p);

  /**
     Adds the tracker coordinates position of a calibration point.

     \b XML-attribute: \c trackerPoint
  */
  void addTrackerPoint(Eigen::Vector3f p);

  /**
     Sets the frequency at which samples are collected when the
     controller button is pressed. Default is 1. At most one sample
     per frame will be used regardless of this value.

     \gmXmlTag{gmTrack,PoseRegistrationEstimator,samplesPerSecond}
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
