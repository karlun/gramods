
#ifndef GRAMODS_TRACK_FLOATTIMESAMPLETRACKER
#define GRAMODS_TRACK_FLOATTIMESAMPLETRACKER

#include <gmTrack/TimeSampleTracker.hh>

#include <gmCore/OFactory.hh>

BEGIN_NAMESPACE_GMTRACK;

/**
   This tracker of float data reports states as specified in the
   samples of the node.

   @see TimeSampleTracker
*/
class FloatTimeSampleTracker : public TimeSampleTracker<float> {

public:

  GM_OFI_DECLARE;
};

END_NAMESPACE_GMTRACK;

#endif
