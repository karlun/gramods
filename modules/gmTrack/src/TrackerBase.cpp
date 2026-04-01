
#include <gmTrack/TrackerBase.hh>

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

void to_json(nlohmann::json &j, const PoseTracker::State &o) {
  j = nlohmann::json {o};
}
void from_json(const nlohmann::json &j, PoseTracker::State &o) {
  o = j.get<std::unordered_map<std::string, PoseTracker::Sample>>();
}

void to_json(nlohmann::json &j, const BinaryTracker::State &o) {
  j = nlohmann::json {o};
}
void from_json(const nlohmann::json &j, BinaryTracker::State &o) {
  o = j.get<std::unordered_map<std::string, BinaryTracker::Sample>>();
}

void to_json(nlohmann::json &j, const FloatTracker::State &o) {
  j = nlohmann::json {o};
}
void from_json(const nlohmann::json &j, FloatTracker::State &o) {
  o = j.get<std::unordered_map<std::string, FloatTracker::Sample>>();
}

void to_json(nlohmann::json &j, const Float2Tracker::State &o) {
  j = nlohmann::json {o};
}
void from_json(const nlohmann::json &j, Float2Tracker::State &o) {
  o = j.get<std::unordered_map<std::string, Float2Tracker::Sample>>();
}

#endif

END_NAMESPACE_GMTRACK;
