
#ifndef GRAMODS_TRACK_TIMESAMPLEPOSETRACKER
#define GRAMODS_TRACK_TIMESAMPLEPOSETRACKER

#include <gmTrack/SinglePoseTracker.hh>
#include <gmCore/io_eigen.hh>
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
  virtual ~TimeSamplePoseTracker();

  /**
     Adds a time sample in seconds, from the start of the program.

     There must be at least two time samples as well as an equal
     amount of time samples and amount of either one of both of
     position and orientation samples.

     \gmXmlTag{gmTrack,TimeSamplePoseTracker,time}
  */
  void addTime(double t);

  /**
     Adds a position sample, in xml as (x y z).

     There must be either no or one position sample, or as many as the
     number of time samples.

     \gmXmlTag{gmTrack,TimeSamplePoseTracker,position}
     @see gramods::operator>>(std::istream &in, Eigen::Vector3f &v)
  */
  void addPosition(Eigen::Vector3f p);

  /**
     Add a orientation sample.

     There must be either no or one orientation sample, or as many as
     the number of time samples.

     \gmXmlTag{gmTrack,TimeSamplePoseTracker,orientation}

     \sa gramods::operator>>(std::istream &, Eigen::Quaternionf &)
  */
  void addOrientation(Eigen::Quaternionf q);

  /**
     Replaces the contents of p with pose data.
  */
  bool getPose(PoseSample &p) override;

  GM_OFI_DECLARE;

private:

  struct Impl;
  std::unique_ptr<Impl> _impl;

};

END_NAMESPACE_GMTRACK;

#endif
