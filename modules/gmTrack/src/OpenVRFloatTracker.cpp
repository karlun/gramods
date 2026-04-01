
#include <gmTrack/OpenVRFloatTracker.hh>

#ifdef gramods_ENABLE_OpenVR

#include <gmCore/Console.hh>
#include <gmCore/RunOnce.hh>
#include <gmCore/TimeTools.hh>

BEGIN_NAMESPACE_GMTRACK;

GM_OFI_DEFINE(OpenVRFloatTracker);
GM_OFI_PARAM2(OpenVRFloatTracker, key, std::string, addKey);
GM_OFI_POINTER2(OpenVRFloatTracker, openVR, gmCore::OpenVR, setOpenVR);

struct OpenVRFloatTracker::Impl {
  std::optional<State> get();

  std::shared_ptr<gmCore::OpenVR> openvr;
  std::vector<std::string> keys;
};

OpenVRFloatTracker::OpenVRFloatTracker()
  : _impl(std::make_unique<Impl>()) {}

OpenVRFloatTracker::~OpenVRFloatTracker() {}

void OpenVRFloatTracker::addKey(std::string name) {
  _impl->keys.push_back(name);
}

void OpenVRFloatTracker::setOpenVR(std::shared_ptr<gmCore::OpenVR> openvr) {
  _impl->openvr = openvr;
}

std::optional<FloatTracker::State> OpenVRFloatTracker::get() {
  return _impl->get();
}

std::optional<FloatTracker::State> OpenVRFloatTracker::Impl::get() {

  if (keys.empty()) {
    GM_RUNONCE(GM_ERR("OpenVRFloatTracker", "No key specified"));
    return std::nullopt;
  }

  if (!openvr) {
    GM_RUNONCE(GM_ERR("OpenVRFloatTracker",
                      "Cannot read float data without OpenVR instance"));
    return std::nullopt;
  }

  State state;
  const auto t0 = openvr->getLastTime();

  for (const auto &key : keys) {

    auto data = openvr->getAnalog(key);
    if (!data) {
      GM_RUNONCE(
          GM_WRN("OpenVRFloatTracker",
                 "Could not get analog data for key '" << key << "'"));
      continue;
    }

    state[key] = {
        .time = t0 - gmCore::TimeTools::secondsToDuration(data->fUpdateTime),
        .value = data->x};
  }

  if (state.empty()) return std::nullopt;
  return state;
}

void OpenVRFloatTracker::traverse(Visitor *visitor) {
  if (_impl->openvr) _impl->openvr->accept(visitor);
}

END_NAMESPACE_GMTRACK;

#endif
