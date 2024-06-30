
#include <gmTrack/OpenVRButtonsTracker.hh>

#ifdef gramods_ENABLE_OpenVR

#include <gmCore/Console.hh>
#include <gmCore/RunOnce.hh>
#include <gmCore/TimeTools.hh>

BEGIN_NAMESPACE_GMTRACK;

GM_OFI_DEFINE(OpenVRButtonsTracker);
GM_OFI_PARAM2(OpenVRButtonsTracker, action, std::string, addAction);
GM_OFI_POINTER2(OpenVRButtonsTracker, openVR, gmCore::OpenVR, setOpenVR);

struct OpenVRButtonsTracker::Impl {
  bool getButtons(ButtonsSample &b);

  std::shared_ptr<gmCore::OpenVR> openvr;
  std::vector<std::string> actions;
};

OpenVRButtonsTracker::OpenVRButtonsTracker()
  : _impl(std::make_unique<Impl>()) {}

OpenVRButtonsTracker::~OpenVRButtonsTracker() {}

void OpenVRButtonsTracker::addAction(std::string name) {
  _impl->actions.push_back(name);
}

void OpenVRButtonsTracker::setOpenVR(std::shared_ptr<gmCore::OpenVR> openvr) {
  _impl->openvr = openvr;
}

bool OpenVRButtonsTracker::getButtons(ButtonsSample &b) {
  return _impl->getButtons(b);
}

bool OpenVRButtonsTracker::Impl::getButtons(ButtonsSample &b) {

  if (actions.empty()) {
    GM_RUNONCE(GM_ERR("OpenVRButtonsTracker", "No action specified"));
    return false;
  }

  if (!openvr) {
    GM_RUNONCE(GM_ERR("OpenVRButtonsTracker",
                      "Cannot read digital data without OpenVR instance"));
    return false;
  }


  ButtonsSample sample;

  float time_offset = std::numeric_limits<float>::max();
  size_t idx = std::numeric_limits<size_t>::max();
  for (const auto &action : actions) {
    ++idx;

    auto data = openvr->getDigital(action);
    if (!data) {
      GM_RUNONCE(
          GM_WRN("OpenVRButtonsTracker",
                 "Could not get digital data for action '" << action << "'"));
      continue;
    }

    sample.buttons[idx] = data->bState;
    time_offset = std::min(time_offset, data->fUpdateTime);
  }

  sample.time =
      openvr->getLastTime() - gmCore::TimeTools::secondsToDuration(time_offset);

  b = sample;
  return true;
}

void OpenVRButtonsTracker::traverse(Visitor *visitor) {
  if (_impl->openvr) _impl->openvr->accept(visitor);
}

END_NAMESPACE_GMTRACK;

#endif
