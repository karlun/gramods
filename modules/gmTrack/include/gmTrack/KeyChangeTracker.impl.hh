
#include <gmTrack/KeyChangeTracker.hh>

#include <gmCore/Console.hh>
#include <gmCore/io_typeid.hh>
#include <gmCore/PreConditionViolation.hh>
#include <gmCore/RunOnce.hh>
#include <gmCore/Updateable.hh>

#include <optional>
#include <unordered_set>

BEGIN_NAMESPACE_GMTRACK;

template <class TYPE> struct KeyChangeTracker<TYPE>::Impl {

  Impl() {}

  std::optional<typename TrackerBase<TYPE>::State> get();

  typedef std::unordered_multimap<std::string, std::string> mappings_t;
  mappings_t mappings;

  std::vector<std::shared_ptr<TrackerBase<TYPE>>> trackers;

  bool retain = false;
};

template <class TYPE>
KeyChangeTracker<TYPE>::KeyChangeTracker()
    : _impl(std::make_unique<KeyChangeTracker<TYPE>::Impl>()) {}

template <class TYPE> KeyChangeTracker<TYPE>::~KeyChangeTracker() {}

template <class TYPE>
void KeyChangeTracker<TYPE>::addMapping(gmCore::string2 m) {
  _impl->mappings.insert({m[0], m[1]});
}

template <class TYPE>
void KeyChangeTracker<TYPE>::addTracker(
    std::shared_ptr<TrackerBase<TYPE>> ptr) {
  _impl->trackers.push_back(ptr);
}

template <class TYPE> void KeyChangeTracker<TYPE>::setRetainMapped(bool on) {
  _impl->retain = on;
}

template <class TYPE>
std::optional<typename TrackerBase<TYPE>::State> KeyChangeTracker<TYPE>::get() {
  return _impl->get();
}

template <class TYPE>
std::optional<typename TrackerBase<TYPE>::State>
KeyChangeTracker<TYPE>::Impl::get() {

  std::optional<typename TrackerBase<TYPE>::State> acc;

  for (auto ptr : trackers) {

    auto state = ptr->get();
    if (!state) continue;

    typename TrackerBase<TYPE>::State rstate;

    for (const auto &s : state.value()) {
      if (!mappings.contains(s.first) || retain) rstate[s.first] = s.second;
      for (auto it = mappings.find(s.first); it != mappings.end(); ++it)
        rstate[it->second] = s.second;
    }

    TrackerBase<TYPE>::merge(acc, rstate);
  }

  return acc;
}

END_NAMESPACE_GMTRACK;
