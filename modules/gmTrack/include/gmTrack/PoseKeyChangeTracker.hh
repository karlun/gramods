
#ifndef GRAMODS_TRACK_POSEKEYCHANGETRACKER
#define GRAMODS_TRACK_POSEKEYCHANGETRACKER

#include <gmTrack/KeyChangeTracker.hh>

#include <gmCore/io_eigen.hh>
#include <gmCore/OFactory.hh>

BEGIN_NAMESPACE_GMTRACK;

/**
   This tracker of gmCore::Pose data reports states from one or more
   other trackers of the same type, optionally also changing the key
   of one or more states.

   @see KeyChangeTracker
*/
class PoseKeyChangeTracker : public KeyChangeTracker<gmCore::Pose> {

public:

  GM_OFI_DECLARE;
};

END_NAMESPACE_GMTRACK;

#endif
