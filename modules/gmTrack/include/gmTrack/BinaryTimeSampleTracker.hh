
#ifndef GRAMODS_TRACK_BINARYTIMESAMPLETRACKER
#define GRAMODS_TRACK_BINARYTIMESAMPLETRACKER

#include <gmTrack/TimeSampleTracker.hh>

#include <gmCore/OFactory.hh>

BEGIN_NAMESPACE_GMTRACK;

/**
   This tracker of binary (bool) data reports states as specified in
   the samples of the node.

   @see TimeSampleTracker
*/
class BinaryTimeSampleTracker : public TimeSampleTracker<bool> {

public:

  GM_OFI_DECLARE;
};

END_NAMESPACE_GMTRACK;

#endif
