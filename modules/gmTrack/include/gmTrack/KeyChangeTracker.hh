
#ifndef GRAMODS_TRACK_KEYCHANGETRACKER
#define GRAMODS_TRACK_KEYCHANGETRACKER

#include <gmTrack/TrackerBase.hh>

#include <gmCore/io_string.hh>
#include <gmCore/OFactory.hh>

BEGIN_NAMESPACE_GMTRACK;

/**
   This tracker reports states from one or more other trackers of the
   same type, optionally also changing the key of one or more states.
*/
template<class TYPE> class KeyChangeTracker : public TrackerBase<TYPE> {

protected:
  struct Impl;
  std::unique_ptr<Impl> _impl;

public:
  KeyChangeTracker();
  virtual ~KeyChangeTracker();

  /**
     Adds a mapping, so that a key that matches the first specified
     string in the specified tracker is changed to the second
     specified string before being returned by get().

     \gmXmlNodeAttr{gmTrack,BinaryKeyChangeTracker,mapping}
     \gmXmlNodeAttr{gmTrack,FloatKeyChangeTracker,mapping}
     \gmXmlNodeAttr{gmTrack,Float2KeyChangeTracker,mapping}
     \gmXmlNodeAttr{gmTrack,PoseKeyChangeTracker,mapping}
  */
  void addMapping(gmCore::string2 map);

  /**
     Adds a tracker to read states from.

     \gmXmlNodeAttr{gmTrack,BinaryKeyChangeTracker,binaryTracker}
     \gmXmlNodeAttr{gmTrack,FloatKeyChangeTracker,floatTracker}
     \gmXmlNodeAttr{gmTrack,Float2KeyChangeTracker,float2Tracker}
     \gmXmlNodeAttr{gmTrack,PoseKeyChangeTracker,poseTracker}
  */
  void addTracker(std::shared_ptr<TrackerBase<TYPE>> ptr);

  /**
     @see TrackerBase::get
  */
  std::optional<typename TrackerBase<TYPE>::State> get() override;
};

END_NAMESPACE_GMTRACK;

#endif
