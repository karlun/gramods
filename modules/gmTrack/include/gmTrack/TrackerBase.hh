
#ifndef GRAMODS_TRACK_TRACKERBASE
#define GRAMODS_TRACK_TRACKERBASE

#include <gmTrack/config.hh>

#include <gmCore/io_float.hh>
#include <gmCore/Object.hh>
#include <gmCore/Pose.hh>

#include <chrono>
#include <optional>

BEGIN_NAMESPACE_GMTRACK;

/**
   The base of trackers of pose, float, float2 or bool data.

   This type of tracker provides a State represented by a map from a
   key string to a tracker sample. Each sample consists of a time when
   this sample was collected and a value of the type of the
   tracker. Observe that the samples in the tracker state may be of
   different age, both because old samples linger in the state and
   potentially because they are collected at different times.

   If a sample is calculated as a combination of two or more other
   samples, then the time stamp should be the latest of that of the
   included samples.
*/
template<class TYPE> class TrackerBase : public gmCore::Object {

public:

  typedef std::chrono::steady_clock clock;

  /**
     Type of a single sample, consisting of the collect time and the
     value.
  */
  struct Sample {
    clock::time_point time; //< Collect time
    TYPE value;             //< Sample value
  };

  /**
     Type of the tracker state.
  */
  typedef std::unordered_map<std::string, Sample> State;

  /**
     Reads state data. Returns nullopt if data could not be read, but
     not if the data are old. Use sample time to check if the data are
     fresh.
  */
  virtual std::optional<State> get() = 0;

  /**
     Returns the default key, in Configuration, for the Object,
     i.e. poseTracker, binaryTracker, floatTracker and float2Tracker,
     respectively.

     @see gmCore::Object::getDefaultKey
  */
  std::string getDefaultKey() override;
};

typedef TrackerBase<gmCore::Pose> PoseTracker;
typedef TrackerBase<bool> BinaryTracker;
typedef TrackerBase<float> FloatTracker;
typedef TrackerBase<gmCore::float2> Float2Tracker;

template<> std::string PoseTracker::getDefaultKey();
template<> std::string BinaryTracker::getDefaultKey();
template<> std::string FloatTracker::getDefaultKey();
template<> std::string Float2Tracker::getDefaultKey();

#ifdef gramods_ENABLE_nlohmann_json
// Should be template but nested type could not be deduced

void to_json(nlohmann::json &j, const PoseTracker::State &o);
void from_json(const nlohmann::json &j, PoseTracker::State &o);

void to_json(nlohmann::json &j, const BinaryTracker::State &o);
void from_json(const nlohmann::json &j, BinaryTracker::State &o);

void to_json(nlohmann::json &j, const FloatTracker::State &o);
void from_json(const nlohmann::json &j, FloatTracker::State &o);

void to_json(nlohmann::json &j, const Float2Tracker::State &o);
void from_json(const nlohmann::json &j, Float2Tracker::State &o);

#endif

END_NAMESPACE_GMTRACK;

#endif
