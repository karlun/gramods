
#ifndef GRAMODS_TRACK_TIMESAMPLEPOSETRACKER
#define GRAMODS_TRACK_TIMESAMPLEPOSETRACKER

#include <gmTrack/SinglePoseTracker.hh>
#include <gmTypes/types.hh>
#include <gmTypes/operators.hh>
#include <gmCore/OFactory.hh>

BEGIN_NAMESPACE_GMTRACK;

/**
   Pose tracker plays back linear interpolation between specified
   samples.
*/
class TimeSamplePoseTracker
  : public SinglePoseTracker {

public:

  TimeSamplePoseTracker();

  /**
     Adds a time sample in seconds, from the start of the
     program. There must be at least two time samples as well as an
     equal amount of time samples and amount of either one of both of
     position and orientation samples.
  */
  void addTime(double t);

  /**
     Adds a position sample. There must be either no or one position
     sample, or as many as the number of time samples.
  */
  void addPosition(gmTypes::float3 p);

  /**
     Adds an orientation sample as a quaternion rotation in format (w
     x y z) where the angle a is expressed in radians. There must be
     either no or one orientation sample, or as many as the number of
     time samples.
  */
  void addQuaternion(gmTypes::float4 r);

  /**
     Adds an orientation sample as an axis angle rotation in format (x
     y z a) where the angle a is expressed in radians. There must be
     either no or one orientation sample, or as many as the number of
     time samples.
  */
  void addAxisAngle(gmTypes::float4 r);

  /**
     Replaces the contents of p with pose data.
  */
  bool getPose(PoseSample &p);

  GM_OFI_DECLARE;

private:

  struct Impl;
  std::unique_ptr<Impl> _impl;

};

END_NAMESPACE_GMTRACK;

#endif
