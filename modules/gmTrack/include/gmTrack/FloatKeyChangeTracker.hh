
#ifndef GRAMODS_TRACK_FLOATKEYCHANGETRACKER
#define GRAMODS_TRACK_FLOATKEYCHANGETRACKER

#include <gmTrack/KeyChangeTracker.hh>

#include <gmCore/io_eigen.hh>
#include <gmCore/OFactory.hh>

BEGIN_NAMESPACE_GMTRACK;

/**
   This tracker of float data reports states from one or more
   other trackers of the same type, optionally also changing the key
   of one or more states.

   @see KeyChangeTracker
*/
class FloatKeyChangeTracker : public KeyChangeTracker<float> {

public:

  GM_OFI_DECLARE;
};

END_NAMESPACE_GMTRACK;

#endif
