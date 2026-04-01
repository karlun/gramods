
#ifndef GRAMODS_TRACK_TIMESAMPLETRACKER
#define GRAMODS_TRACK_TIMESAMPLETRACKER

#include <gmTrack/TrackerBase.hh>

#include <gmCore/OFactory.hh>

BEGIN_NAMESPACE_GMTRACK;

/**
   This tracker reports states as specified in the samples of the
   node.

   The samples can be either timed or time-less. For time-less samples
   no time can be specified and the key and states are either
   specified in pairs (key1, state1, key2, state2, ...) or one key for
   all states (key, state1, state2, ...). For timed samples, either
   specify one key and then one state for each time (key, state1,
   time1, state2, time2, ...) or one key for each sample (keyA,
   sampleA1, keyB, sampleB1, time1, keyA, sampleA2, keyB, sampleB2,
   ...).

   Interpolatable samples (e.g. float) are interpolated between a
   sample and the next, while boolean states are left constant until
   the next sample.
*/
template<class TYPE> class TimeSampleTracker : public TrackerBase<TYPE> {

protected:
  struct Impl;
  std::unique_ptr<Impl> _impl;

public:
  TimeSampleTracker();
  virtual ~TimeSampleTracker();

  /**
     Adds a time sample in seconds, from the start of the
     program. First add keys and states and then specify the time
     for the sample containing those key states.

     \gmXmlNodeAttr{gmTrack,BinaryTimeSampleTracker,time}
     \gmXmlNodeAttr{gmTrack,FloatTimeSampleTracker,time}
     \gmXmlNodeAttr{gmTrack,Float2TimeSampleTracker,time}
     \gmXmlNodeAttr{gmTrack,PoseTimeSampleTracker,time}
  */
  void addTime(double seconds);

  /**
     Adds an key string for a sample. This is a string associated with
     an input, e.g. "main_button". Use addTime ("time" in XML) to
     set a time and add the sample.

     \gmXmlNodeAttr{gmTrack,BinaryTimeSampleTracker,key}
     \gmXmlNodeAttr{gmTrack,FloatTimeSampleTracker,key}
     \gmXmlNodeAttr{gmTrack,Float2TimeSampleTracker,key}
     \gmXmlNodeAttr{gmTrack,PoseTimeSampleTracker,key}
  */
  void addKey(std::string key);

  /**
     Adds the value for a sample. Use addTime ("time" in XML) to set a
     time and add the sample.

     \gmXmlNodeAttr{gmTrack,BinaryTimeSampleTracker,value}
     \gmXmlNodeAttr{gmTrack,FloatTimeSampleTracker,value}
     \gmXmlNodeAttr{gmTrack,Float2TimeSampleTracker,value}
     \gmXmlNodeAttr{gmTrack,PoseTimeSampleTracker,value}
  */
  void addValue(TYPE state);

  void addKeyValue(std::string key, TYPE value) {
    addKey(key);
    addValue(value);
  }

  /**
     @see PoseTracker::get
  */
  std::optional<typename TrackerBase<TYPE>::State> get() override;
};

END_NAMESPACE_GMTRACK;

#endif
