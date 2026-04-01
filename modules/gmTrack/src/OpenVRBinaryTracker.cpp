
#include <gmTrack/OpenVRBinaryTracker.hh>

#ifdef gramods_ENABLE_OpenVR

#include <gmCore/Console.hh>
#include <gmCore/RunOnce.hh>
#include <gmCore/TimeTools.hh>

BEGIN_NAMESPACE_GMTRACK;

GM_OFI_DEFINE(OpenVRBinaryTracker);
GM_OFI_PARAM2(OpenVRBinaryTracker, key, std::string, addKey);
GM_OFI_POINTER2(OpenVRBinaryTracker, openVR, gmCore::OpenVR, setOpenVR);

struct OpenVRBinaryTracker::Impl {
  std::optional<State> get();

  std::shared_ptr<gmCore::OpenVR> openvr;
  std::vector<std::string> keys;
};

OpenVRBinaryTracker::OpenVRBinaryTracker()
  : _impl(std::make_unique<Impl>()) {}

OpenVRBinaryTracker::~OpenVRBinaryTracker() {}

void OpenVRBinaryTracker::addKey(std::string name) {
  _impl->keys.push_back(name);
}

void OpenVRBinaryTracker::setOpenVR(std::shared_ptr<gmCore::OpenVR> openvr) {
  _impl->openvr = openvr;
}

std::optional<BinaryTracker::State> OpenVRBinaryTracker::get() {
  return _impl->get();
}

std::optional<BinaryTracker::State> OpenVRBinaryTracker::Impl::get() {

  if (keys.empty()) {
    GM_RUNONCE(GM_ERR("OpenVRBinaryTracker", "No key specified"));
    return std::nullopt;
  }

  if (!openvr) {
    GM_RUNONCE(GM_ERR("OpenVRBinaryTracker",
                      "Cannot read digital data without OpenVR instance"));
    return std::nullopt;
  }

  State state;
  const auto t0 = openvr->getLastTime();

  for (const auto &key : keys) {
    auto data = openvr->getDigital(key);
    if (!data) {
      GM_RUNONCE(
          GM_WRN("OpenVRBinaryTracker",
                 "Could not get digital data for key '" << key << "'"));
      continue;
    }

    state[key] = {
        .time = t0 - gmCore::TimeTools::secondsToDuration(data->fUpdateTime),
        .value = data->bState};
  }

  if (state.empty()) return std::nullopt;
  return state;
}

void OpenVRBinaryTracker::traverse(Visitor *visitor) {
  if (_impl->openvr) _impl->openvr->accept(visitor);
}

END_NAMESPACE_GMTRACK;

#endif
