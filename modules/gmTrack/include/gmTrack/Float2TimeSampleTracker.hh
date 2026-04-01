
#ifndef GRAMODS_TRACK_FLOAT2TIMESAMPLETRACKER
#define GRAMODS_TRACK_FLOAT2TIMESAMPLETRACKER

#include <gmTrack/TimeSampleTracker.hh>

#include <gmCore/io_float.hh>
#include <gmCore/OFactory.hh>

BEGIN_NAMESPACE_GMTRACK;

/**
   This tracker of gmCore::float2 (std::array<float, 2>) data reports
   states as specified in the samples of the node.

   @see TimeSampleTracker
*/
class Float2TimeSampleTracker : public TimeSampleTracker<gmCore::float2> {

public:

  GM_OFI_DECLARE;
};

END_NAMESPACE_GMTRACK;

#endif
