
#include <gmTrack/VrpnTracker.hh>

#include <gmCore/Console.hh>
#include <gmCore/io_typeid.hh>
#include <gmCore/PreConditionViolation.hh>
#include <gmCore/RunOnce.hh>
#include <gmCore/Updateable.hh>

#include <optional>
#include <unordered_set>

using namespace std::literals::string_literals;

BEGIN_NAMESPACE_GMTRACK;

template<class TYPE, class vrpn_TRACKER, class vrpn_CB>
struct VrpnTracker<TYPE, vrpn_TRACKER, vrpn_CB>::Impl : gmCore::Updateable {

  typedef typename TrackerBase<TYPE>::State State;

  Impl() : gmCore::Updateable(10) {}

  void update(clock::time_point now, size_t frame) override;

  void handler(const std::string key,
               const std::string connection_str,
               const vrpn_CB info);

  void setState(State &state,
                clock::time_point now,
                const std::string &key,
                const vrpn_CB info);

  struct RemoteData {
    RemoteData(VrpnTracker<TYPE, vrpn_TRACKER, vrpn_CB>::Impl *parent, std::string id);

    static void VRPN_CALLBACK handler(void *userdata, const vrpn_CB info);

    VrpnTracker<TYPE, vrpn_TRACKER, vrpn_CB>::Impl *parent;
    std::unique_ptr<vrpn_TRACKER> remote;

    std::string connection_str;
    std::string key;
  };

  std::vector<std::unique_ptr<VrpnTracker<TYPE, vrpn_TRACKER, vrpn_CB>::Impl::RemoteData>> trackers;

  std::optional<typename TrackerBase<TYPE>::State> state;
  bool got_data;

  const std::string type_str =
      GM_STR("VrpnTracker<" << demangle(typeid(TYPE)) << ">");
};

template<class TYPE, class vrpn_TRACKER, class vrpn_CB>
VrpnTracker<TYPE, vrpn_TRACKER, vrpn_CB>::VrpnTracker()
  : _impl(std::make_unique<VrpnTracker<TYPE, vrpn_TRACKER, vrpn_CB>::Impl>()) {}

template<class TYPE, class vrpn_TRACKER, class vrpn_CB> VrpnTracker<TYPE, vrpn_TRACKER, vrpn_CB>::~VrpnTracker() {}

template<class TYPE, class vrpn_TRACKER, class vrpn_CB>
void VrpnTracker<TYPE, vrpn_TRACKER, vrpn_CB>::Impl::update(
    clock::time_point now, size_t frame) {

  trackers.erase(
      std::remove_if(
          trackers.begin(),
          trackers.end(),
          [](const std::unique_ptr<
              VrpnTracker<TYPE, vrpn_TRACKER, vrpn_CB>::Impl::RemoteData> &ptr)
              -> bool {
            if (ptr->remote->connectionPtr()->doing_okay()) return false;
            GM_ERR("VrpnTracker<...>",
                   "Defunct connection (" << ptr->connection_str
                                          << "); closing vrpn connection");
            // TODO: Remove state
            return true;
          }),
      trackers.end());

  if (trackers.empty()) {
    GM_RUNONCE(GM_WRN(type_str, "No remotes to read data from"));
    return;
  }

  do {
    got_data = false;
    for (auto &tracker : trackers) tracker->remote->mainloop();
  } while (got_data);
}

template<class TYPE, class vrpn_TRACKER, class vrpn_CB>
void VrpnTracker<TYPE, vrpn_TRACKER, vrpn_CB>::addConnectionString(std::string id) {
  _impl->trackers.push_back(
      std::make_unique<
          typename VrpnTracker<TYPE, vrpn_TRACKER, vrpn_CB>::Impl::RemoteData>(
          _impl.get(), id));
}

template<class TYPE, class vrpn_TRACKER, class vrpn_CB>
VrpnTracker<TYPE, vrpn_TRACKER, vrpn_CB>::Impl::RemoteData::RemoteData(
    VrpnTracker<TYPE, vrpn_TRACKER, vrpn_CB>::Impl *parent, std::string id)
  : parent(parent),
    remote(std::make_unique<vrpn_TRACKER>(id.c_str())),
    connection_str(id) {
  key = id.substr(0, id.find("@"));
  if (key.empty() || key[0] != '/') key = "/"s + key;
  remote->register_change_handler(
      this,
      VrpnTracker<TYPE, vrpn_TRACKER, vrpn_CB>::Impl::RemoteData::handler);
}

template<class TYPE, class vrpn_TRACKER, class vrpn_CB>
void VRPN_CALLBACK
VrpnTracker<TYPE, vrpn_TRACKER, vrpn_CB>::Impl::RemoteData::handler(
    void *ptr, const vrpn_CB info) {
  auto *_this =
      static_cast<VrpnTracker<TYPE, vrpn_TRACKER, vrpn_CB>::Impl::RemoteData *>(
          ptr);
  _this->parent->handler(_this->key, _this->connection_str, info);
}

template<class TYPE, class vrpn_TRACKER, class vrpn_CB>
void VrpnTracker<TYPE, vrpn_TRACKER, vrpn_CB>::Impl::handler(
    const std::string key,
    const std::string connection_str,
    const vrpn_CB info) {

  typedef std::chrono::steady_clock clock;

  auto secs = std::chrono::duration_cast<clock::duration>(
      std::chrono::seconds(info.msg_time.tv_sec));
  auto usecs = std::chrono::duration_cast<clock::duration>(
      std::chrono::microseconds(info.msg_time.tv_usec));
  auto time = clock::time_point(secs + usecs);

  if (!state) state = typename TrackerBase<TYPE>::State {};

  setState(state.value(), time, key, info);
  got_data = true;
}

template<class TYPE, class vrpn_TRACKER, class vrpn_CB>
std::optional<typename TrackerBase<TYPE>::State>
VrpnTracker<TYPE, vrpn_TRACKER, vrpn_CB>::get() {
  return _impl->state;
}

END_NAMESPACE_GMTRACK;
