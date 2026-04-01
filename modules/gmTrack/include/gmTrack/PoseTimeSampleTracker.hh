
#ifndef GRAMODS_TRACK_POSETIMESAMPLETRACKER
#define GRAMODS_TRACK_POSETIMESAMPLETRACKER

#include <gmTrack/TimeSampleTracker.hh>

#include <gmCore/io_eigen.hh>
#include <gmCore/OFactory.hh>

BEGIN_NAMESPACE_GMTRACK;

/**
   This tracker of gmCore::Pose data reports states as specified in
   the samples of the node.

   @see TimeSampleTracker
*/
class PoseTimeSampleTracker : public TimeSampleTracker<gmCore::Pose> {

public:

  GM_OFI_DECLARE;
};

END_NAMESPACE_GMTRACK;

#endif
