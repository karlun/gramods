
#include <gmTrack/KeyChangeTracker.hh>

#include <gmCore/Console.hh>
#include <gmCore/io_typeid.hh>
#include <gmCore/PreConditionViolation.hh>
#include <gmCore/RunOnce.hh>
#include <gmCore/Updateable.hh>

#include <optional>
#include <unordered_set>

BEGIN_NAMESPACE_GMTRACK;

template<class TYPE> struct KeyChangeTracker<TYPE>::Impl {

  Impl() {}

  std::optional<typename TrackerBase<TYPE>::State> get();

  typedef std::unordered_map<std::string, std::string> mappings_t;
  mappings_t mappings;

  std::vector<std::shared_ptr<TrackerBase<TYPE>>> trackers;
};

template<class TYPE>
KeyChangeTracker<TYPE>::KeyChangeTracker()
  : _impl(std::make_unique<KeyChangeTracker<TYPE>::Impl>()) {}

template<class TYPE> KeyChangeTracker<TYPE>::~KeyChangeTracker() {}

template<class TYPE>
void KeyChangeTracker<TYPE>::addMapping(gmCore::string2 m) {
  _impl->mappings[m[0]] = m[1];
}

template<class TYPE>
void KeyChangeTracker<TYPE>::addTracker(
    std::shared_ptr<TrackerBase<TYPE>> ptr) {
  _impl->trackers.push_back(ptr);
}

template<class TYPE>
std::optional<typename TrackerBase<TYPE>::State>
KeyChangeTracker<TYPE>::get() {
  return _impl->get();
}

template<class TYPE>
std::optional<typename TrackerBase<TYPE>::State>
KeyChangeTracker<TYPE>::Impl::get() {

  std::optional<typename TrackerBase<TYPE>::State> acc;

  for (auto ptr : trackers) {

    auto state = ptr->get();
    if (!state) continue;

    typename TrackerBase<TYPE>::State rstate;

    for (const auto &s : state.value()) {
      if (mappings.contains(s.first)) {
        rstate[mappings.at(s.first)] = s.second;
      } else {
        rstate[s.first] = s.second;
      }
    }

    if (!acc) [[unlikely]] {
      acc = rstate;
    } else {
      acc->merge(rstate);
    }
  }

  return acc;
}

END_NAMESPACE_GMTRACK;
