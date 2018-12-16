
#ifndef GRAMODS_TRACK_TIMESAMPLEANALOGSTRACKER
#define GRAMODS_TRACK_TIMESAMPLEANALOGSTRACKER

#include <gmTrack/AnalogsTracker.hh>

#include <gmTypes/all.hh>
#include <gmCore/OFactory.hh>

BEGIN_NAMESPACE_GMTRACK;

/**
   This analogs tracker reports analog states as specified in the
   samples of the node.
 */
class TimeSampleAnalogsTracker
  : public gmTrack::AnalogsTracker {

public:

  TimeSampleAnalogsTracker();

  /**
     Adds a time sample in seconds, from the start of the
     program. There must be at least two time samples as well as an
     equal amount of time samples and amount of samples.
  */
  void addTime(double t);

  /**
     Adds an analogs sample as three floats representing three analog inputs.
  */
  void addAnalogs(gmTypes::float3 a);

  /**
     Replaces the contents of p with analog data.
  */
  bool getAnalogs(AnalogsSample &a);

  GM_OFI_DECLARE;

private:

  struct Impl;
  std::unique_ptr<Impl> _impl;

};

END_NAMESPACE_GMTRACK;

#endif
