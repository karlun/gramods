
#ifndef GRAMODS_TRACK_FLOAT2KEYCHANGETRACKER
#define GRAMODS_TRACK_FLOAT2KEYCHANGETRACKER

#include <gmTrack/KeyChangeTracker.hh>

#include <gmCore/io_float.hh>
#include <gmCore/OFactory.hh>

BEGIN_NAMESPACE_GMTRACK;

/**
   This tracker of gmCore::float2 data reports states from one or more
   other trackers of the same type, optionally also changing the key
   of one or more states.

   @see KeyChangeTracker
*/
class Float2KeyChangeTracker : public KeyChangeTracker<gmCore::float2> {

public:

  GM_OFI_DECLARE;
};

END_NAMESPACE_GMTRACK;

#endif
