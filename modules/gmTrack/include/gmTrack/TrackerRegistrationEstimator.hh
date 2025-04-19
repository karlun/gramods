
#ifndef GM_TRACK_TRACKERREGISTRATIONESTIMATOR
#define GM_TRACK_TRACKERREGISTRATIONESTIMATOR

#include <gmTrack/PoseSampleCollector.hh>

BEGIN_NAMESPACE_GMTRACK;

/**
   The TrackerRegistrationEstimator is a utility for estimating the base
   of a tracking system's coordinates, i e registration of the
   tracking system into room or display system coordinates.

   The utility is used by marking, with the tracking device, the
   position of known, pre-specified positions in the room or display
   system.
*/
class TrackerRegistrationEstimator
  : public PoseSampleCollector {

public:

  TrackerRegistrationEstimator();
  ~TrackerRegistrationEstimator();

  /**
     Adds a known calibration point, in room coordinates.

     \gmXmlTag{gmTrack,TrackerRegistrationEstimator,actualPosition}
  */
  void addActualPosition(Eigen::Vector3f p);

  /**
     Force registration estimation. This is needed only if the
     component is used non-interactively.
  */
  void performRegistration();

  /**
     Extract registration matrix, either raw or without
     scaling. Returns true if there is a registration, false
     otherwise.
  */
  bool getRegistration(Eigen::Matrix4f * RAW, Eigen::Matrix4f * UNIT);

  GM_OFI_DECLARE;

private:
  struct Impl;
};

END_NAMESPACE_GMTRACK;

#endif
