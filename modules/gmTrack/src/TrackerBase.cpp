
#include <gmTrack/TrackerBase.hh>

#include <gmCore/TimeTools.hh>

#ifdef gramods_ENABLE_nlohmann_json
#include <nlohmann/json.hpp>
#endif

BEGIN_NAMESPACE_GMTRACK;

template<> std::string PoseTracker::getDefaultKey() {
  return "poseTracker";
}

template<> std::string BinaryTracker::getDefaultKey() {
  return "binaryTracker";
}

template<> std::string FloatTracker::getDefaultKey() {
  return "floatTracker";
}

template<> std::string Float2Tracker::getDefaultKey() {
  return "float2Tracker";
}

#ifdef gramods_ENABLE_nlohmann_json

#if 0
// Should be template but nested type could not be deduced
namespace {
template<class TYPE>
void _to_json(nlohmann::json &j, const typename TrackerBase<TYPE>::State &o) {
  j = nlohmann::json {o};
}

template<class TYPE>
void _from_json(const nlohmann::json &j,
                typename TrackerBase<TYPE>::State &o) {
  o = j.get<std::unordered_map<std::string, 
                               TrackerBase<TYPE>::Sample>>();
}
}

#endif

void to_json(nlohmann::json &j, const PoseTracker::Sample &o) {
  j = nlohmann::json{{"time", gmCore::TimeTools::timePointToSeconds(o.time)},
                     {"value", o.value}};
}
void from_json(const nlohmann::json &j, PoseTracker::Sample &o) {
  o.time = gmCore::TimeTools::secondsToTimePoint(j.get<double>());
  o.value = j.get<gmCore::Pose>();
}

void to_json(nlohmann::json &j, const BinaryTracker::Sample &o) {
  j = nlohmann::json{{"time", gmCore::TimeTools::timePointToSeconds(o.time)},
                     {"value", o.value}};
}
void from_json(const nlohmann::json &j, BinaryTracker::Sample &o) {
  o.time = gmCore::TimeTools::secondsToTimePoint(j.get<double>());
  o.value = j.get<bool>();
}

void to_json(nlohmann::json &j, const FloatTracker::Sample &o) {
  j = nlohmann::json{{"time", gmCore::TimeTools::timePointToSeconds(o.time)},
                     {"value", o.value}};
}
void from_json(const nlohmann::json &j, FloatTracker::Sample &o) {
  o.time = gmCore::TimeTools::secondsToTimePoint(j.get<double>());
  o.value = j.get<float>();
}

void to_json(nlohmann::json &j, const Float2Tracker::Sample &o) {
  j = nlohmann::json{{"time", gmCore::TimeTools::timePointToSeconds(o.time)},
                     {"value", o.value}};
}
void from_json(const nlohmann::json &j, Float2Tracker::Sample &o) {
  o.time = gmCore::TimeTools::secondsToTimePoint(j.get<double>());
  o.value = j.get<gmCore::float2>();
}

#endif

END_NAMESPACE_GMTRACK;
