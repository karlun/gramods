
#include <gmTrack/OpenVRAnalogsTracker.hh>

#ifdef gramods_ENABLE_OpenVR

#include <gmCore/Console.hh>
#include <gmCore/RunOnce.hh>
#include <gmCore/TimeTools.hh>

BEGIN_NAMESPACE_GMTRACK;

GM_OFI_DEFINE(OpenVRAnalogsTracker);
GM_OFI_PARAM2(OpenVRAnalogsTracker, action, std::string, addAction);
GM_OFI_POINTER2(OpenVRAnalogsTracker, openVR, gmCore::OpenVR, setOpenVR);

struct OpenVRAnalogsTracker::Impl {
  bool getAnalogs(AnalogsSample &b);

  std::shared_ptr<gmCore::OpenVR> openvr;
  std::vector<std::string> actions;
};

OpenVRAnalogsTracker::OpenVRAnalogsTracker()
  : _impl(std::make_unique<Impl>()) {}

OpenVRAnalogsTracker::~OpenVRAnalogsTracker() {}

void OpenVRAnalogsTracker::addAction(std::string name) {
  _impl->actions.push_back(name);
}

void OpenVRAnalogsTracker::setOpenVR(std::shared_ptr<gmCore::OpenVR> openvr) {
  _impl->openvr = openvr;
}

bool OpenVRAnalogsTracker::getAnalogs(AnalogsSample &b) {
  return _impl->getAnalogs(b);
}

bool OpenVRAnalogsTracker::Impl::getAnalogs(AnalogsSample &b) {

  if (actions.empty()) {
    GM_RUNONCE(GM_ERR("OpenVRAnalogsTracker", "No action specified"));
    return false;
  }

  if (!openvr) {
    GM_RUNONCE(GM_ERR("OpenVRAnalogsTracker",
                      "Cannot read analog data without OpenVR instance"));
    return false;
  }


  AnalogsSample sample;
  sample.analogs.reserve(3 * actions.size());

  float time_offset = std::numeric_limits<float>::max();
  for (const auto &action : actions) {

    auto data = openvr->getAnalog(action);
    if (!data) {
      GM_RUNONCE(
          GM_WRN("OpenVRAnalogsTracker",
                 "Could not get analog data for action '" << action << "'"));
      continue;
    }

    sample.analogs.push_back(data->x);
    sample.analogs.push_back(data->y);
    sample.analogs.push_back(data->z);
    time_offset = std::min(time_offset, data->fUpdateTime);
  }

  sample.time =
      openvr->getLastTime() - gmCore::TimeTools::secondsToDuration(time_offset);

  b = sample;
  return true;
}

void OpenVRAnalogsTracker::traverse(Visitor *visitor) {
  if (_impl->openvr) _impl->openvr->accept(visitor);
}

END_NAMESPACE_GMTRACK;

#endif
